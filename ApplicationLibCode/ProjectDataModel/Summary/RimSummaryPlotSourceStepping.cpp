/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimSummaryPlotSourceStepping.h"

#include "RiaEnsembleNameTools.h"
#include "RiaGuiApplication.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"

#include "RimDataSourceSteppingTools.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryDataSourceStepping.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotControls.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiToolBarEditor.h"

#include <QString>

#include <algorithm>
#include <vector>

namespace caf
{
template <>
void AppEnum<RimSummaryPlotSourceStepping::SourceSteppingDimension>::setUp()
{
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::VECTOR, "VECTOR", "Vector" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::WELL, "WELL", "Well" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::GROUP, "GROUP", "Group" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::REGION, "REGION", "Region" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::BLOCK, "BLOCK", "Block" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::AQUIFER, "AQUIFER", "Aquifer" );
    setDefault( RimSummaryPlotSourceStepping::SourceSteppingDimension::VECTOR );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping()
    : m_sourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS )
{
    CAF_PDM_InitObject( "Summary Curves Modifier" );

    CAF_PDM_InitFieldNoDefault( &m_stepDimension, "StepDimension", "Step Dimension" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveCase", "Case" );

    CAF_PDM_InitField( &m_includeEnsembleCasesForCaseStepping,
                       "IncludeEnsembleCasesForCaseStepping",
                       true,
                       "Include Ensemble Cases in Case List" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );
    CAF_PDM_InitFieldNoDefault( &m_groupName, "GroupName", "Group Name" );
    CAF_PDM_InitFieldNoDefault( &m_region, "Region", "Region" );
    CAF_PDM_InitFieldNoDefault( &m_vectorName, "VectorName", "Vector" );

    CAF_PDM_InitFieldNoDefault( &m_cellBlock, "CellBlock", "Block" );
    CAF_PDM_InitFieldNoDefault( &m_segment, "Segment", "Segment" );
    CAF_PDM_InitFieldNoDefault( &m_completion, "Completion", "Completion" );
    CAF_PDM_InitFieldNoDefault( &m_aquifer, "Aquifer", "Aquifer" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );

    CAF_PDM_InitFieldNoDefault( &m_placeholderForLabel, "Placeholder", "" );
    m_placeholderForLabel = "No common identifiers detected";
    m_placeholderForLabel.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_placeholderForLabel.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_indexLabel, "IndexLabel", QString( "Step By" ), "Step By" );
    m_indexLabel.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_indexLabel.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setSourceSteppingType( RimSummaryDataSourceStepping::Axis sourceSteppingType )
{
    m_sourceSteppingType = sourceSteppingType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setSourceSteppingObject( caf::PdmObject* sourceObject )
{
    m_objectForSourceStepping = sourceObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextStep()
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    modifyCurrentIndex( valueField, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevStep()
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    modifyCurrentIndex( valueField, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::fieldsToShowInToolbar()
{
    return toolbarFieldsForDataSourceStepping();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto visible = activeFieldsForDataSourceStepping();
    if ( visible.empty() )
    {
        uiOrdering.add( &m_placeholderForLabel );
    }

    for ( auto f : visible )
    {
        uiOrdering.add( f );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryPlotSourceStepping::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( ( fieldNeedingOptions == &m_includeEnsembleCasesForCaseStepping ) || ( fieldNeedingOptions == &m_stepDimension ) )
    {
        return caf::PdmObject::calculateValueOptions( fieldNeedingOptions );
    }

    if ( ( fieldNeedingOptions == &m_placeholderForLabel ) || ( fieldNeedingOptions == &m_indexLabel ) )
    {
        return options;
    }

    if ( fieldNeedingOptions == &m_summaryCase )
    {
        auto summaryCases = RimSummaryPlotSourceStepping::summaryCasesForSourceStepping();
        for ( auto sumCase : summaryCases )
        {
            if ( sumCase->ensemble() )
            {
                if ( m_includeEnsembleCasesForCaseStepping() )
                {
                    auto name = sumCase->ensemble()->name() + " : " + sumCase->displayCaseName();
                    options.append( caf::PdmOptionItemInfo( name, sumCase ) );
                }
            }
            else
            {
                options.append( caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase ) );
            }
        }

        return options;
    }

    if ( fieldNeedingOptions == &m_ensemble )
    {
        RimProject* proj = RimProject::current();
        for ( auto ensemble : proj->summaryGroups() )
        {
            if ( ensemble->isEnsemble() )
            {
                options.append( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
            }
        }

        return options;
    }

    auto addresses = adressesForSourceStepping();
    if ( !addresses.empty() )
    {
        if ( fieldNeedingOptions == &m_vectorName )
        {
            std::map<QString, QString> displayAndValueStrings = optionsForQuantity( addresses );

            for ( const auto& displayAndValue : displayAndValueStrings )
            {
                options.append( caf::PdmOptionItemInfo( displayAndValue.first, displayAndValue.second ) );
            }

            if ( options.isEmpty() )
            {
                options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
            }
        }
        else
        {
            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_INVALID;
            std::string                                  secondaryIdentifier;

            if ( fieldNeedingOptions == &m_wellName )
            {
                category = RifEclipseSummaryAddress::SUMMARY_WELL;
            }
            else if ( fieldNeedingOptions == &m_region )
            {
                category = RifEclipseSummaryAddress::SUMMARY_REGION;
            }
            else if ( fieldNeedingOptions == &m_groupName )
            {
                category = RifEclipseSummaryAddress::SUMMARY_GROUP;
            }
            else if ( fieldNeedingOptions == &m_cellBlock )
            {
                category = RifEclipseSummaryAddress::SUMMARY_BLOCK;
            }
            else if ( fieldNeedingOptions == &m_segment )
            {
                secondaryIdentifier = m_wellName().toStdString();
                category            = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
            }
            else if ( fieldNeedingOptions == &m_completion )
            {
                secondaryIdentifier = m_wellName().toStdString();
                category            = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
            }
            else if ( fieldNeedingOptions == &m_aquifer )
            {
                category = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
            }

            std::vector<QString> identifierTexts;

            if ( category != RifEclipseSummaryAddress::SUMMARY_INVALID )
            {
                RiaSummaryAddressAnalyzer analyzer;
                analyzer.appendAddresses( addresses );

                identifierTexts = analyzer.identifierTexts( category, secondaryIdentifier );
            }

            if ( !identifierTexts.empty() )
            {
                for ( const auto& text : identifierTexts )
                {
                    options.append( caf::PdmOptionItemInfo( text, text ) );
                }
            }
            else
            {
                options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                     const QVariant&            oldValue,
                                                     const QVariant&            newValue )
{
    std::vector<RimSummaryCurve*> curves;
    if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->allCurves( m_sourceSteppingType );

    if ( changedField == &m_stepDimension )
    {
        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();
        return;
    }

    if ( changedField == &m_includeEnsembleCasesForCaseStepping )
    {
        RimSummaryCurveCollection* curveCollection = nullptr;
        this->firstAncestorOrThisOfType( curveCollection );
        if ( curveCollection )
        {
            curveCollection->updateConnectedEditors();
        }

        RimEnsembleCurveSetCollection* ensembleCurveColl = nullptr;
        this->firstAncestorOrThisOfType( ensembleCurveColl );
        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();

        return;
    }

    bool triggerLoadDataAndUpdate = false;

    if ( changedField == &m_summaryCase )
    {
        if ( m_summaryCase() )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCase*                       previousCase = dynamic_cast<RimSummaryCase*>( variantHandle.p() );

            for ( auto curve : curves )
            {
                if ( isYAxisStepping() )
                {
                    if ( previousCase == curve->summaryCaseY() )
                    {
                        curve->setSummaryCaseY( m_summaryCase );
                        curve->setCurveAppearanceFromCaseType();
                    }
                }

                if ( isXAxisStepping() )
                {
                    if ( previousCase == curve->summaryCaseX() )
                    {
                        curve->setSummaryCaseX( m_summaryCase );
                    }
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_groupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_vectorName.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_ensemble )
    {
        if ( m_ensemble() && dataSourceSteppingObject() )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCaseCollection* previousCollection = dynamic_cast<RimSummaryCaseCollection*>( variantHandle.p() );

            for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
            {
                if ( curveSet->summaryCaseCollection() == previousCollection )
                {
                    curveSet->setSummaryCaseCollection( m_ensemble );
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_groupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_vectorName.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_vectorName )
    {
        for ( auto curve : curves )
        {
            if ( isYAxisStepping() )
            {
                auto adr = curve->summaryAddressY();
                if ( RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, &adr ) )
                    curve->setSummaryAddressY( adr );
            }

            if ( isXAxisStepping() )
            {
                auto adr = curve->summaryAddressX();
                if ( RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, &adr ) )
                    curve->setSummaryAddressX( adr );
            }

            curve->setDefaultCurveAppearance();
        }

        if ( dataSourceSteppingObject() )
        {
            for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
            {
                auto adr = curveSet->summaryAddress();
                if ( RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, &adr ) )
                    curveSet->setSummaryAddress( adr );
            }
        }
        m_vectorName.uiCapability()->updateConnectedEditors();
        triggerLoadDataAndUpdate = true;
    }

    if ( changedField != &m_vectorName )
    {
        RifEclipseSummaryAddress::SummaryVarCategory summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_INVALID;
        if ( changedField == &m_wellName )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL;
        }
        else if ( changedField == &m_region )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_REGION;
        }
        else if ( changedField == &m_groupName )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_GROUP;
        }
        else if ( changedField == &m_cellBlock )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_BLOCK;
        }
        else if ( changedField == &m_segment )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
        }
        else if ( changedField == &m_completion )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
        }
        else if ( changedField == &m_aquifer )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
        }

        if ( summaryCategoryToModify != RifEclipseSummaryAddress::SUMMARY_INVALID )
        {
            for ( auto curve : curves )
            {
                if ( isYAxisStepping() )
                {
                    RifEclipseSummaryAddress adr = curve->summaryAddressY();
                    RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curve->setSummaryAddressY( adr );
                }

                if ( isXAxisStepping() )
                {
                    RifEclipseSummaryAddress adr = curve->summaryAddressX();
                    RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curve->setSummaryAddressX( adr );
                }
            }

            if ( dataSourceSteppingObject() )
            {
                for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
                {
                    auto adr = curveSet->summaryAddress();
                    RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curveSet->setSummaryAddress( adr );
                }
            }

            triggerLoadDataAndUpdate = true;
        }
    }

    if ( triggerLoadDataAndUpdate )
    {
        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfType( summaryPlot );

        RimSummaryMultiPlot* summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( m_objectForSourceStepping.p() );
        if ( summaryMultiPlot )
        {
            summaryMultiPlot->updatePlots();
            summaryMultiPlot->updatePlotWindowTitle();
            summaryMultiPlot->zoomAll();
            RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateMultiPlotToolBar();
        }
        else
        {
            summaryPlot->updatePlotTitle();
            summaryPlot->loadDataAndUpdate();
            summaryPlot->updateConnectedEditors();
            summaryPlot->curvesChanged.send();
        }

        RimEnsembleCurveSetCollection* ensembleCurveColl = nullptr;
        this->firstAncestorOrThisOfType( ensembleCurveColl );
        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>( summaryPlot );
        if ( summaryCrossPlot )
        {
            // Trigger update of curve collection (and summary toolbar in main window), as the visibility of combo
            // boxes might have been changed due to the updates in this function
            RimSummaryCurveCollection* curveCollection = nullptr;
            this->firstAncestorOrThisOfType( curveCollection );
            if ( curveCollection )
            {
                curveCollection->updateConnectedEditors();
            }

            RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateMultiPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmValueField* RimSummaryPlotSourceStepping::fieldToModify()
{
    switch ( m_stepDimension() )
    {
        case SourceSteppingDimension::SUMMARY_CASE:
            return &m_summaryCase;
            break;

        case SourceSteppingDimension::ENSEMBLE:
            return &m_ensemble;

        case SourceSteppingDimension::WELL:
            return &m_wellName;

        case SourceSteppingDimension::GROUP:
            return &m_groupName;

        case SourceSteppingDimension::REGION:
            return &m_region;

        case SourceSteppingDimension::VECTOR:
            return &m_vectorName;

        case SourceSteppingDimension::BLOCK:
            return &m_cellBlock;

        case SourceSteppingDimension::AQUIFER:
            return &m_aquifer;

        default:
            break;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::adressesForSourceStepping() const
{
    std::set<RifEclipseSummaryAddress> addressSet;

    if ( dataSourceSteppingObject() )
    {
        for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
        {
            if ( curveSet && curveSet->summaryCaseCollection() )
            {
                auto addresses = curveSet->summaryCaseCollection()->ensembleSummaryAddresses();
                addressSet.insert( addresses.begin(), addresses.end() );
            }
        }

        std::vector<RimSummaryCurve*> curves;
        if ( dataSourceSteppingObject() )
            curves = dataSourceSteppingObject()->curvesForStepping( m_sourceSteppingType );

        for ( auto curve : curves )
        {
            if ( !curve ) continue;

            if ( isYAxisStepping() && curve->summaryCaseY() && curve->summaryCaseY()->summaryReader() )
            {
                auto addresses = curve->summaryCaseY()->summaryReader()->allResultAddresses();
                addressSet.insert( addresses.begin(), addresses.end() );
            }

            if ( isXAxisStepping() && curve->summaryCaseX() && curve->summaryCaseX()->summaryReader() )
            {
                auto addresses = curve->summaryCaseX()->summaryReader()->allResultAddresses();
                addressSet.insert( addresses.begin(), addresses.end() );
            }
        }
    }

    return addressSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::addressesForCurvesInPlot() const
{
    std::set<RifEclipseSummaryAddress> addresses;

    if ( dataSourceSteppingObject() )
    {
        for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
        {
            addresses.insert( curveSet->summaryAddress() );
        }

        std::vector<RimSummaryCurve*> curves;
        if ( dataSourceSteppingObject() )
            curves = dataSourceSteppingObject()->curvesForStepping( m_sourceSteppingType );

        for ( auto curve : curves )
        {
            if ( isYAxisStepping() )
            {
                addresses.insert( curve->summaryAddressY() );
            }

            if ( isXAxisStepping() )
            {
                addresses.insert( curve->summaryAddressX() );
            }
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimSummaryPlotSourceStepping::summaryCasesCurveCollection() const
{
    std::set<RimSummaryCase*> sumCases;

    std::vector<RimSummaryCurve*> curves;
    if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->curvesForStepping( m_sourceSteppingType );
    for ( auto c : curves )
    {
        if ( isYAxisStepping() )
        {
            sumCases.insert( c->summaryCaseY() );
        }

        if ( isXAxisStepping() )
        {
            sumCases.insert( c->summaryCaseX() );
        }
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::activeFieldsForDataSourceStepping()
{
    RimProject* proj = RimProject::current();
    if ( !proj ) return {};

    std::vector<caf::PdmFieldHandle*> fields;

    fields.push_back( &m_stepDimension );

    auto sumCases = summaryCasesCurveCollection();
    if ( sumCases.size() == 1 )
    {
        if ( proj->allSummaryCases().size() > 1 )
        {
            m_summaryCase = *( sumCases.begin() );

            fields.push_back( &m_summaryCase );
            fields.push_back( &m_includeEnsembleCasesForCaseStepping );
        }
    }

    auto ensembleColl = ensembleCollection();
    if ( ensembleColl.size() == 1 )
    {
        if ( proj->summaryGroups().size() > 1 )
        {
            m_ensemble = *( ensembleColl.begin() );

            fields.push_back( &m_ensemble );
        }
    }

    std::vector<caf::PdmFieldHandle*> fieldsCommonForAllCurves;

    {
        RiaSummaryAddressAnalyzer analyzer;
        analyzer.appendAddresses( addressesForCurvesInPlot() );

        RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_INVALID;

        if ( !analyzer.categories().empty() )
        {
            if ( analyzer.categories().size() == 1 )
            {
                category = *( analyzer.categories().begin() );
            }
            else
            {
                bool allCategoriesAreDependingOnWellName = true;
                for ( auto c : analyzer.categories() )
                {
                    if ( !RifEclipseSummaryAddress::isDependentOnWellName( c ) )
                    {
                        allCategoriesAreDependingOnWellName = false;
                    }
                }

                if ( allCategoriesAreDependingOnWellName )
                {
                    category = RifEclipseSummaryAddress::SUMMARY_WELL;
                }
            }
        }

        if ( category != RifEclipseSummaryAddress::SUMMARY_INVALID )
        {
            if ( analyzer.wellNames().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.wellNames().begin() ) );
                m_wellName  = txt;

                fieldsCommonForAllCurves.push_back( &m_wellName );
            }

            if ( analyzer.groupNames().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.groupNames().begin() ) );
                m_groupName = txt;

                fieldsCommonForAllCurves.push_back( &m_groupName );
            }

            if ( analyzer.regionNumbers().size() == 1 )
            {
                m_region = *( analyzer.regionNumbers().begin() );

                fieldsCommonForAllCurves.push_back( &m_region );
            }

            if ( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt = QString::number( *( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).begin() ) );
                m_segment   = txt;

                fieldsCommonForAllCurves.push_back( &m_segment );
            }

            if ( analyzer.blocks().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.blocks().begin() ) );
                m_cellBlock = txt;

                fieldsCommonForAllCurves.push_back( &m_cellBlock );
            }

            if ( analyzer.wellCompletions( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.wellCompletions( m_wellName().toStdString() ).begin() ) );
                m_completion = txt;

                fieldsCommonForAllCurves.push_back( &m_completion );
            }

            if ( analyzer.aquifers().size() == 1 )
            {
                m_aquifer = *( analyzer.aquifers().begin() );

                fieldsCommonForAllCurves.push_back( &m_aquifer );
            }

            if ( !analyzer.quantityNameForTitle().empty() )
            {
                QString txt  = QString::fromStdString( analyzer.quantityNameForTitle() );
                m_vectorName = txt;

                fieldsCommonForAllCurves.push_back( &m_vectorName );
            }
        }
    }

    for ( const auto& f : fieldsCommonForAllCurves )
    {
        fields.push_back( f );
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCaseCollection*> RimSummaryPlotSourceStepping::ensembleCollection() const
{
    std::set<RimSummaryCaseCollection*> summaryCaseCollections;

    if ( dataSourceSteppingObject() )
    {
        for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
        {
            if ( curveSet && curveSet->summaryCaseCollection() )
            {
                summaryCaseCollections.insert( curveSet->summaryCaseCollection() );
            }
        }
    }

    return summaryCaseCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isXAxisStepping() const
{
    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS ) return true;

    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::X_AXIS ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isYAxisStepping() const
{
    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS ) return true;

    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::Y_AXIS ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::modifyCurrentIndex( caf::PdmValueField* valueField, int indexOffset, bool notifyChange )
{
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions( valueField );
    RimDataSourceSteppingTools::modifyCurrentIndex( valueField, options, indexOffset, notifyChange );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryPlotSourceStepping::summaryCasesForSourceStepping()
{
    std::vector<RimSummaryCase*> cases;

    RimProject* proj = RimProject::current();
    for ( auto sumCase : proj->allSummaryCases() )
    {
        if ( sumCase->isObservedData() ) continue;

        RimSummaryCaseCollection* sumCaseColl = nullptr;
        sumCase->firstAncestorOrThisOfType( sumCaseColl );

        if ( sumCaseColl && sumCaseColl->isEnsemble() )
        {
            if ( m_includeEnsembleCasesForCaseStepping() )
            {
                cases.push_back( sumCase );
            }
        }
        else
        {
            cases.push_back( sumCase );
        }
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDataSourceStepping* RimSummaryPlotSourceStepping::dataSourceSteppingObject() const
{
    return dynamic_cast<RimSummaryDataSourceStepping*>( m_objectForSourceStepping.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
    if ( myAttr )
    {
        if ( field == &m_stepDimension )
        {
            myAttr->showPreviousAndNextButtons = false;
        }
        else
        {
            QString nextText       = RimSummaryPlotControls::nextStepKeyText();
            QString prevText       = RimSummaryPlotControls::prevStepKeyText();
            myAttr->nextButtonText = "Next (" + nextText + ")";
            myAttr->prevButtonText = "Previous (" + prevText + ")";

            myAttr->nextIcon     = QIcon( ":/ComboBoxDown.svg" );
            myAttr->previousIcon = QIcon( ":/ComboBoxUp.svg" );

            myAttr->showPreviousAndNextButtons = true;
        }
    }

    if ( myAttr && ( uiConfigName == caf::PdmUiToolBarEditor::uiEditorConfigName() ) )
    {
        myAttr->minimumWidth = 120;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::toolbarFieldsForDataSourceStepping()
{
    std::vector<caf::PdmFieldHandle*> fields;
    fields.push_back( &m_indexLabel );
    fields.push_back( &m_stepDimension );

    caf::PdmFieldHandle* field = fieldToModify();
    if ( field != nullptr ) fields.push_back( field );

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryPlotSourceStepping::stepAddress( RifEclipseSummaryAddress addr, int direction )
{
    auto                      addresses = adressesForSourceStepping();
    RiaSummaryAddressAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    switch ( m_stepDimension() )
    {
        case SourceSteppingDimension::WELL:
        {
            auto  ids     = analyzer.identifierTexts( RifEclipseSummaryAddress::SUMMARY_WELL, "" );
            auto& curName = addr.wellName();
            auto  found   = std::find( ids.begin(), ids.end(), QString::fromStdString( curName ) );
            if ( found != ids.end() )
            {
                if ( direction > 0 )
                {
                    found++;
                }
                else
                {
                    if ( found != ids.begin() ) found--;
                }
                if ( found != ids.end() ) addr.setWellName( ( *found ).toStdString() );
            }
        }
        break;

        case SourceSteppingDimension::GROUP:
        {
            auto  ids     = analyzer.identifierTexts( RifEclipseSummaryAddress::SUMMARY_GROUP, "" );
            auto& curName = addr.groupName();
            auto  found   = std::find( ids.begin(), ids.end(), QString::fromStdString( curName ) );
            if ( found != ids.end() )
            {
                if ( direction > 0 )
                {
                    found++;
                }
                else
                {
                    if ( found != ids.begin() ) found--;
                }
                if ( found != ids.end() ) addr.setGroupName( ( *found ).toStdString() );
            }
        }
        break;

        case SourceSteppingDimension::REGION:
        {
            auto ids       = analyzer.identifierTexts( RifEclipseSummaryAddress::SUMMARY_REGION, "" );
            int  curRegion = addr.regionNumber();
            auto found     = std::find( ids.begin(), ids.end(), curRegion );
            if ( found != ids.end() )
            {
                if ( direction > 0 )
                {
                    found++;
                }
                else
                {
                    if ( found != ids.begin() ) found--;
                }
                if ( found != ids.end() ) addr.setRegion( ( *found ).toInt() );
            }
        }
        break;

        case SourceSteppingDimension::VECTOR:
        {
            auto options = optionsForQuantity( addresses );

            std::vector<QString> values;
            for ( auto it = options.begin(); it != options.end(); it++ )
            {
                values.push_back( it->second );
            }

            QString qName = QString::fromStdString( addr.vectorName() );
            auto    found = std::find( values.begin(), values.end(), qName );
            if ( found != values.end() )
            {
                if ( direction > 0 )
                {
                    found++;
                }
                else
                {
                    if ( found != values.begin() ) found--;
                }
                if ( found != values.end() ) addr.setVectorName( ( *found ).toStdString() );
            }
        }
        break;

        case SourceSteppingDimension::BLOCK:
        {
            auto ids     = analyzer.identifierTexts( RifEclipseSummaryAddress::SUMMARY_BLOCK, "" );
            auto curName = addr.blockAsString();
            auto found   = std::find( ids.begin(), ids.end(), QString::fromStdString( curName ) );
            if ( found != ids.end() )
            {
                if ( direction > 0 )
                {
                    found++;
                }
                else
                {
                    if ( found != ids.begin() ) found--;
                }
                if ( found != ids.end() )
                {
                    addr.setCellIjk( ( *found ).toStdString() );
                }
            }
        }
        break;

        case SourceSteppingDimension::AQUIFER:
        {
            auto ids       = analyzer.identifierTexts( RifEclipseSummaryAddress::SUMMARY_AQUIFER, "" );
            int  curRegion = addr.aquiferNumber();
            auto found     = std::find( ids.begin(), ids.end(), curRegion );
            if ( found != ids.end() )
            {
                if ( direction > 0 )
                {
                    found++;
                }
                else
                {
                    if ( found != ids.begin() ) found--;
                }
                if ( found != ids.end() ) addr.setAquiferNumber( ( *found ).toInt() );
            }
        }
        break;

        default:
            break;
    }
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::syncWithStepper( RimSummaryPlotSourceStepping* other )
{
    switch ( m_stepDimension() )
    {
        case SourceSteppingDimension::SUMMARY_CASE:
            m_summaryCase = other->m_summaryCase();
            break;

        case SourceSteppingDimension::ENSEMBLE:
            m_ensemble = other->m_ensemble();
            break;

        case SourceSteppingDimension::WELL:
            m_wellName = other->m_wellName();
            break;

        case SourceSteppingDimension::GROUP:
            m_groupName = other->m_groupName();
            break;

        case SourceSteppingDimension::REGION:
            m_region = other->m_region();
            break;

        case SourceSteppingDimension::VECTOR:
            m_vectorName = other->m_vectorName();
            break;

        case SourceSteppingDimension::BLOCK:
            m_cellBlock = other->m_cellBlock();
            break;

        case SourceSteppingDimension::AQUIFER:
            m_aquifer = other->m_aquifer();
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimSummaryPlotSourceStepping::optionsForQuantity( std::set<RifEclipseSummaryAddress> addresses )
{
    RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_FIELD;

    auto visibleCurveAddresses = addressesForCurvesInPlot();
    if ( !visibleCurveAddresses.empty() )
    {
        category = visibleCurveAddresses.begin()->category();
    }

    std::map<QString, QString> displayAndValueStrings;

    {
        RiaSummaryAddressAnalyzer quantityAnalyzer;

        auto subset = RiaSummaryAddressAnalyzer::addressesForCategory( addresses, category );
        quantityAnalyzer.appendAddresses( subset );

        RiaSummaryAddressAnalyzer analyzerForVisibleCurves;
        analyzerForVisibleCurves.appendAddresses( visibleCurveAddresses );

        auto quantities = quantityAnalyzer.quantities();
        for ( const auto& s : quantities )
        {
            QString valueString = QString::fromStdString( s );

            displayAndValueStrings[valueString] = valueString;
        }
    }

    return displayAndValueStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::SourceSteppingDimension RimSummaryPlotSourceStepping::stepDimension() const
{
    return m_stepDimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setStepDimension( SourceSteppingDimension dimension )
{
    m_stepDimension = dimension;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryPlotSourceStepping::stepCase( int direction )
{
    std::vector<RimSummaryCase*> cases;

    auto summaryCases = RimSummaryPlotSourceStepping::summaryCasesForSourceStepping();
    for ( auto sumCase : summaryCases )
    {
        if ( sumCase->ensemble() )
        {
            if ( m_includeEnsembleCasesForCaseStepping() )
            {
                cases.push_back( sumCase );
            }
        }
        else
        {
            cases.push_back( sumCase );
        }
    }

    auto found = std::find( cases.begin(), cases.end(), m_summaryCase() );
    if ( found != cases.end() )
    {
        if ( direction > 0 )
        {
            found++;
        }
        else
        {
            if ( found != cases.begin() ) found--;
        }
        if ( found != cases.end() ) return *found;
    }

    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimSummaryPlotSourceStepping::stepEnsemble( int direction )
{
    std::vector<RimSummaryCaseCollection*> ensembles;

    RimProject* proj = RimProject::current();
    for ( auto ensemble : proj->summaryGroups() )
    {
        if ( ensemble->isEnsemble() )
        {
            ensembles.push_back( ensemble );
        }
    }

    auto found = std::find( ensembles.begin(), ensembles.end(), m_ensemble() );
    if ( found != ensembles.end() )
    {
        if ( direction > 0 )
        {
            found++;
        }
        else
        {
            if ( found != ensembles.begin() ) found--;
        }
        if ( found != ensembles.end() ) return *found;
    }

    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::updateStepIndex( int direction )
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    bool notifyChange = false;
    modifyCurrentIndex( valueField, direction, notifyChange );
}
