/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "cafCmdFeature.h"

class RimFaultInViewCollection;
class RimFaultRASettings;
class RimSurfaceCollection;

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RicRunFaultReactAssessmentFeature : public caf::CmdFeature
{
protected:
    RicRunFaultReactAssessmentFeature();

    virtual RimFaultInViewCollection* faultCollection();

    int selectedFaultID();
    int faultIDFromName( QString faultname ) const;

    RimSurfaceCollection* surfaceCollection();

    bool runPostProcessing( int faultID, RimFaultRASettings* settings );
    void reloadSurfaces( RimFaultRASettings* settings );

    void addParameterFileForCleanUp( QString filename );
    void cleanUpParameterFiles();

    void removeFile( QString filename );

    void runBasicProcessing( int faultID );
    void runAdvancedProcessing( int faultID );

private:
    std::list<QString> m_parameterFilesToCleanUp;
};
