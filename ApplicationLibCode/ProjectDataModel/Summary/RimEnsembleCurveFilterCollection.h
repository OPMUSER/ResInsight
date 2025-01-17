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

#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class RimEnsembleCurveFilter;
class RiuTextContentFrame;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleCurveFilterCollection();

    RimEnsembleCurveFilter*              addFilter( const QString& ensembleParameterName = "" );
    std::vector<RimEnsembleCurveFilter*> filters() const;

    bool isActive() const;
    int  countActiveFilters() const;
    void loadDataAndUpdate();

    QString              filterDescriptions() const;
    RiuTextContentFrame* makeFilterDescriptionFrame() const;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /* = "" */ ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* objectToggleField() override;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    caf::PdmField<bool>                              m_active;
    caf::PdmChildArrayField<RimEnsembleCurveFilter*> m_filters;
    caf::PdmField<bool>                              m_newFilterButton;
};
