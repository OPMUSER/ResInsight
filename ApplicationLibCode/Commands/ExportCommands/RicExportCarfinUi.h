/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RicCellRangeUi;

namespace caf
{
class VecIjk;
}

//==================================================================================================
///
//==================================================================================================
class RicExportCarfinUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicExportCarfinUi();

    void setCase( RimEclipseCase* rimCase );

    int                   maxWellCount() const;
    caf::VecIjk           lgrCellCount() const;
    const RicCellRangeUi* cellRange() const;
    QString               exportFileName() const;
    RimEclipseCase*       caseToApply() const;
    QString               gridName() const;

private:
    void setCasePointers( RimEclipseCase* rimCase );
    void setDefaultValuesFromCase();

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<QString>              m_exportFileName;
    caf::PdmPtrField<RimEclipseCase*>   m_caseToApply;
    caf::PdmChildField<RicCellRangeUi*> m_cellRange;

    caf::PdmField<int> m_cellCountI;
    caf::PdmField<int> m_cellCountJ;
    caf::PdmField<int> m_cellCountK;

    caf::PdmField<int> m_maxWellCount;
};
