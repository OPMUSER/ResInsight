#include "RivContourMapProjectionPartMgr.h"

#include "RiaWeightedMeanCalculator.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivScalarMapperUtils.h"

#include "RimContourMapView.h"
#include "RimContourMapProjection.h"

#include "cafEffectGenerator.h"

#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryUtils.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapProjectionPartMgr::RivContourMapProjectionPartMgr(RimContourMapProjection* contourMapProjection, RimContourMapView* contourMap)
{
    m_contourMapProjection = contourMapProjection;
    m_parentContourMap = contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendProjectionToModel(cvf::ModelBasicList* model, const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::DrawableGeo> drawable = createDrawable(displayCoordTransform);
    if (drawable.notNull() && drawable->boundingBox().isValid())
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawable.p());

        cvf::ref<cvf::Vec2fArray> textureCoords = createTextureCoords();
        cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();
        RivScalarMapperUtils::applyTextureResultsToPart(part.p(), textureCoords.p(), mapper, 1.0f, caf::FC_NONE, true, m_parentContourMap->backgroundColor());

        part->setSourceInfo(new RivObjectSourceInfo(m_contourMapProjection.p()));

        model->addPart(part.p());
    }

    if (m_contourMapProjection->showContourLines())
    {
        std::vector<cvf::ref<cvf::DrawableGeo>> contourDrawables = createContourPolygons(displayCoordTransform);
        for (cvf::ref<cvf::DrawableGeo> contourDrawable : contourDrawables)
        {
            if (contourDrawable.notNull() && contourDrawable->boundingBox().isValid())
            {
                caf::MeshEffectGenerator meshEffectGen(cvf::Color3::BLACK);
                meshEffectGen.setLineWidth(1.0f);
                meshEffectGen.createAndConfigurePolygonOffsetRenderState(caf::PO_2);
                cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setDrawable(contourDrawable.p());
                part->setEffect(effect.p());
                part->setSourceInfo(new RivMeshLinesSourceInfo(m_contourMapProjection.p()));

                model->addPart(part.p());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec2fArray> RivContourMapProjectionPartMgr::createTextureCoords() const
{
    cvf::Vec2ui patchSize = m_contourMapProjection->surfaceGridSize();

    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray(m_contourMapProjection->vertexCount());

    for (uint j = 0; j < patchSize.y(); ++j)
    {
        for (uint i = 0; i < patchSize.x(); ++i)
        {
            if (m_contourMapProjection->hasResultAt(i, j))
            {
                double value = m_contourMapProjection->value(i, j);
                (*textureCoords)[i + j * patchSize.x()] =
                    m_contourMapProjection->legendConfig()->scalarMapper()->mapToTextureCoord(value);
            }
            else
            {
                (*textureCoords)[i + j * patchSize.x()] = cvf::Vec2f(1.0, 1.0);
            }
        }
    }
    return textureCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::removeTrianglesWithNoResult(cvf::UIntArray* vertices) const
{
    std::vector<cvf::uint> trianglesWithResult;
    
    for (size_t n = 0; n < vertices->size(); n += 3)
    {
        bool anyInvalid = false;
        for (size_t t = 0; !anyInvalid && t < 3; ++t)
        {
            cvf::uint vertexNumber = (*vertices)[n + t];
            cvf::Vec2ui ij = m_contourMapProjection->ijFromGridIndex(vertexNumber);
            if (!m_contourMapProjection->hasResultAt(ij.x(), ij.y()))
            {
                anyInvalid = true;
            }
        }
        for (size_t t = 0; !anyInvalid && t < 3; ++t)
        {
            cvf::uint vertexNumber = (*vertices)[n + t];
            trianglesWithResult.push_back(vertexNumber);
        }        
    }
    (*vertices) = cvf::UIntArray(trianglesWithResult);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivContourMapProjectionPartMgr::createDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray;
    m_contourMapProjection->generateVertices(vertexArray.p(), displayCoordTransform);
    cvf::Vec2ui patchSize = m_contourMapProjection->surfaceGridSize();

    // Surface
    cvf::ref<cvf::UIntArray> faceList = new cvf::UIntArray;
    cvf::GeometryUtils::tesselatePatchAsTriangles(patchSize.x(), patchSize.y(), 0u, true, faceList.p());    

    removeTrianglesWithNoResult(faceList.p());

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLES, faceList.p());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->addPrimitiveSet(indexUInt.p());
    geo->setVertexArray(vertexArray.p());
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<cvf::DrawableGeo>> RivContourMapProjectionPartMgr::createContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    RimContourMapProjection::ContourPolygons contourPolygons = m_contourMapProjection->generateContourPolygons(displayCoordTransform);

    std::vector<cvf::ref<cvf::DrawableGeo>> contourDrawables;
    contourDrawables.reserve(contourPolygons.size());
    for (size_t i = 0; i < contourPolygons.size(); ++i)
    {
        cvf::ref<cvf::Vec3fArray> vertexArray = contourPolygons[i];
        std::vector<cvf::uint> indices;
        indices.reserve(contourPolygons[i]->size());
        for (cvf::uint j = 0; j < contourPolygons[i]->size(); ++j)
        {
            indices.push_back(j);
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);
        indexedUInt->setIndices(indexArray.p());

        cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

        geo->addPrimitiveSet(indexedUInt.p());
        geo->setVertexArray(vertexArray.p());
        contourDrawables.push_back(geo);
    }
    return contourDrawables;
}