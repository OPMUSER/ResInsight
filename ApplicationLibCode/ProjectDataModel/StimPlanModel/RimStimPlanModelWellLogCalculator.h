/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelPropertyCalculator.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"
#include "RiaStimPlanModelDefines.h"

#include "cvfObject.h"

#include <vector>

class RigEclipseCaseData;
class RigResultAccessor;
class RimEclipseInputPropertyCollection;
class RimEclipseResultDefinition;
class RimEclipseCase;

class RimStimPlanModelWellLogCalculator : public RimStimPlanModelPropertyCalculator
{
public:
    RimStimPlanModelWellLogCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator );

    bool calculate( RiaDefines::CurveProperty curveProperty,
                    const RimStimPlanModel*   stimPlanModel,
                    int                       timeStep,
                    std::vector<double>&      values,
                    std::vector<double>&      measuredDepthValues,
                    std::vector<double>&      tvDepthValues,
                    double&                   rkbDiff ) const override;

    bool isMatching( RiaDefines::CurveProperty curveProperty ) const override;

    static const std::vector<double>& loadResults( RigEclipseCaseData*           caseData,
                                                   RiaDefines::PorosityModelType porosityModel,
                                                   RiaDefines::ResultCatType     resultType,
                                                   const QString&                propertyName );

protected:
    static bool hasMissingValues( const std::vector<double>& values );
    static void replaceMissingValues( std::vector<double>& values, double defaultValue );
    static void replaceMissingValues( std::vector<double>& values, const std::vector<double>& replacementValues );
    cvf::ref<RigResultAccessor> findMissingValuesAccessor( RigEclipseCaseData*                caseData,
                                                           RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                           int                                gridIndex,
                                                           int                                timeStepIndex,
                                                           const QString&                     resultName ) const;

    void addOverburden( RiaDefines::CurveProperty curveProperty,
                        const RimStimPlanModel*   stimPlanModel,
                        std::vector<double>&      tvDepthValues,
                        std::vector<double>&      measuredDepthValues,
                        std::vector<double>&      values ) const;

    void addUnderburden( RiaDefines::CurveProperty curveProperty,
                         const RimStimPlanModel*   stimPlanModel,
                         std::vector<double>&      tvDepthValues,
                         std::vector<double>&      measuredDepthValues,
                         std::vector<double>&      values ) const;

    static void scaleByNetToGross( const RimStimPlanModel*    stimPlanModel,
                                   const std::vector<double>& netToGross,
                                   std::vector<double>&       values );

    virtual bool extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                           const RimStimPlanModel*   stimPlanModel,
                                           int                       timeStep,
                                           std::vector<double>&      values,
                                           std::vector<double>&      measuredDepthValues,
                                           std::vector<double>&      tvDepthValues,
                                           double&                   rkbDiff ) const;

    bool extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                   const RimStimPlanModel*   stimPlanModel,
                                   RimEclipseCase*           eclipseCase,
                                   RiaDefines::ResultCatType resultCategory,
                                   const QString             resultVariable,
                                   int                       timeStep,
                                   std::vector<double>&      values,
                                   std::vector<double>&      measuredDepthValues,
                                   std::vector<double>&      tvDepthValues,
                                   double&                   rkbDiff ) const;

    bool extractValuesForPropertyWithConfigurations( RiaDefines::CurveProperty curveProperty,
                                                     const RimStimPlanModel*   stimPlanModel,
                                                     int                       timeStep,
                                                     std::vector<double>&      values,
                                                     std::vector<double>&      measuredDepthValues,
                                                     std::vector<double>&      tvDepthValues,
                                                     double&                   rkbDiff ) const;

    bool replaceMissingValuesWithDefault( RiaDefines::CurveProperty curveProperty,
                                          const RimStimPlanModel*   stimPlanModel,
                                          std::vector<double>&      values,
                                          std::vector<double>&      measuredDepthValues,
                                          std::vector<double>&      tvDepthValues,
                                          double&                   rkbDiff ) const;

    bool replaceMissingValuesWithOtherProperty( RiaDefines::CurveProperty curveProperty,
                                                const RimStimPlanModel*   stimPlanModel,
                                                int                       timeStep,
                                                std::vector<double>&      values ) const;

    bool replaceMissingValuesWithOtherKLayer( RiaDefines::CurveProperty  curveProperty,
                                              const RimStimPlanModel*    stimPlanModel,
                                              int                        timeStep,
                                              const std::vector<double>& measuredDepths,
                                              const std::vector<double>& tvDepthValues,
                                              std::vector<double>&       values,
                                              int                        kDirection ) const;

    RimStimPlanModelCalculator* m_stimPlanModelCalculator;
};
