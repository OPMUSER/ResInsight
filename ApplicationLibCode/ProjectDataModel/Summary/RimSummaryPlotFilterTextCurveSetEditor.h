/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

using SummarySource = caf::PdmObject;
class RifEclipseSummaryAddress;

class RimSummaryPlotFilterTextCurveSetEditor : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotFilterTextCurveSetEditor();
    ~RimSummaryPlotFilterTextCurveSetEditor() override;

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

    static QString curveFilterFieldKeyword();
    void           updateTextFilter();

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void setupBeforeSave() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    static void appendOptionItemsForSources( QList<caf::PdmOptionItemInfo>& options );

    std::vector<SummarySource*> selectedSummarySources() const;
    QString                     curveFilterTextWithoutOutdatedLabel() const;

    void updateParentPlot();

    static std::set<RifEclipseSummaryAddress> addressesForSource( SummarySource* summarySource );

    void insertFilteredAddressesInSet( const QStringList&                        curveFilters,
                                       const std::set<RifEclipseSummaryAddress>& allAddressesInCase,
                                       std::set<RifEclipseSummaryAddress>*       setToInsertFilteredAddressesIn,
                                       std::vector<bool>*                        usedFilters );

    static QString curveFilterRecentlyUsedRegistryKey();

private:
    caf::PdmPtrArrayField<SummarySource*> m_selectedSources;

    caf::PdmField<QString> m_curveFilterLabelText;
    caf::PdmField<QString> m_curveFilterText;
    caf::PdmField<bool>    m_includeDiffCurves;

    bool m_isFieldRecentlyChangedFromGui;
};
