/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RicRunAdvFaultReactAssessment3dFeature.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRunAdvFaultReactAssessment3dFeature, "RicRunAdvFaultReactAssessment3dFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunAdvFaultReactAssessment3dFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();
    if ( userData.isNull() || userData.type() != QVariant::String ) return;

    QString faultName = userData.toString();

    int faultID = faultIDFromName( faultName );
    if ( faultID >= 0 ) runAdvancedProcessing( faultID );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunAdvFaultReactAssessment3dFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Run Advanced Processing" );
}
