////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicSummaryPlotBuilder.h"

#include "PlotTemplateCommands/RicSummaryPlotTemplateTools.h"
#include "SummaryPlotCommands/RicNewSummaryEnsembleCurveSetFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryTools.h"

#include "RifEclipseSummaryAddress.h"
#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RiaPreferencesSummary.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotBuilder::RicSummaryPlotBuilder()
    : m_individualPlotPerDataSource( false )
    , m_graphCurveGrouping( RicSummaryPlotBuilder::RicGraphCurveGrouping::NONE )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setDataSources( const std::vector<RimSummaryCase*>&           summaryCases,
                                            const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    m_summaryCases = summaryCases;
    m_ensembles    = ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setAddresses( const std::set<RifEclipseSummaryAddress>& addresses )
{
    m_addresses = addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setIndividualPlotPerDataSource( bool enable )
{
    m_individualPlotPerDataSource = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setGrouping( RicGraphCurveGrouping groping )
{
    m_graphCurveGrouping = groping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicSummaryPlotBuilder::createPlots() const
{
    std::vector<RimSummaryPlot*> plots;

    if ( m_individualPlotPerDataSource )
    {
        if ( m_graphCurveGrouping == RicGraphCurveGrouping::SINGLE_CURVES )
        {
            for ( const auto& adr : m_addresses )
            {
                for ( auto summaryCase : m_summaryCases )
                {
                    auto plot = createPlot( { adr }, { summaryCase }, {} );
                    plots.push_back( plot );
                }

                for ( auto ensemble : m_ensembles )
                {
                    auto plot = createPlot( { adr }, {}, { ensemble } );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::CURVES_FOR_OBJECT )
        {
            RiaSummaryAddressAnalyzer analyzer;
            analyzer.appendAddresses( m_addresses );

            auto groups = analyzer.addressesGroupedByObject();
            for ( const auto& group : groups )
            {
                std::set<RifEclipseSummaryAddress> addresses;
                addresses.insert( group.begin(), group.end() );

                for ( auto summaryCase : m_summaryCases )
                {
                    auto plot = createPlot( addresses, { summaryCase }, {} );
                    plots.push_back( plot );
                }

                for ( auto ensemble : m_ensembles )
                {
                    auto plot = createPlot( addresses, {}, { ensemble } );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::NONE )
        {
            for ( auto summaryCase : m_summaryCases )
            {
                auto plot = createPlot( m_addresses, { summaryCase }, {} );
                plots.push_back( plot );
            }

            for ( auto ensemble : m_ensembles )
            {
                auto plot = createPlot( m_addresses, {}, { ensemble } );
                plots.push_back( plot );
            }
        }
    }
    else // all data sources in same plot
    {
        if ( m_graphCurveGrouping == RicGraphCurveGrouping::SINGLE_CURVES )
        {
            for ( const auto& adr : m_addresses )
            {
                if ( !m_summaryCases.empty() )
                {
                    auto plot = createPlot( { adr }, m_summaryCases, {} );
                    plots.push_back( plot );
                }

                if ( !m_ensembles.empty() )
                {
                    auto plot = createPlot( { adr }, {}, m_ensembles );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::CURVES_FOR_OBJECT )
        {
            RiaSummaryAddressAnalyzer analyzer;
            analyzer.appendAddresses( m_addresses );

            auto groups = analyzer.addressesGroupedByObject();
            for ( const auto& group : groups )
            {
                std::set<RifEclipseSummaryAddress> addresses;
                addresses.insert( group.begin(), group.end() );

                if ( !m_summaryCases.empty() )
                {
                    auto plot = createPlot( addresses, m_summaryCases, {} );
                    plots.push_back( plot );
                }

                if ( !m_ensembles.empty() )
                {
                    auto plot = createPlot( addresses, {}, m_ensembles );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::NONE )
        {
            if ( !m_summaryCases.empty() )
            {
                auto plot = createPlot( m_addresses, m_summaryCases, {} );
                plots.push_back( plot );
            }

            if ( !m_ensembles.empty() )
            {
                auto plot = createPlot( m_addresses, {}, m_ensembles );
                plots.push_back( plot );
            }
        }
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryPlotBuilder::addressesForSource( caf::PdmObject* summarySource )
{
    auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( summarySource );
    if ( ensemble )
    {
        return ensemble->ensembleSummaryAddresses();
    }

    auto sumCase = dynamic_cast<RimSummaryCase*>( summarySource );
    if ( sumCase )
    {
        auto reader = sumCase ? sumCase->summaryReader() : nullptr;
        if ( reader )
        {
            return reader->allResultAddresses();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RicSummaryPlotBuilder::createCurveSet( RimSummaryCaseCollection*       ensemble,
                                                            const RifEclipseSummaryAddress& addr )
{
    auto curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryCaseCollection( ensemble );
    curveSet->setSummaryAddress( addr );

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicSummaryPlotBuilder::createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr )
{
    auto curve = new RimSummaryCurve();

    curve->setSummaryCaseY( summaryCase );
    curve->setSummaryAddressY( addr );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RicSummaryPlotBuilder::duplicatePlots( const std::vector<RimPlot*>& sourcePlots )
{
    std::vector<RimPlot*> plots;

    for ( auto plot : sourcePlots )
    {
        auto copy = dynamic_cast<RimPlot*>( plot->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

        {
            // TODO: Workaround for fixing the PdmPointer in RimEclipseResultDefinition
            //    caf::PdmPointer<RimEclipseCase> m_eclipseCase;
            // This pdmpointer must be changed to a ptrField

            auto saturationPressurePlotOriginal = dynamic_cast<RimSaturationPressurePlot*>( plot );
            auto saturationPressurePlotCopy     = dynamic_cast<RimSaturationPressurePlot*>( copy );
            if ( saturationPressurePlotCopy && saturationPressurePlotOriginal )
            {
                RimSaturationPressurePlot::fixPointersAfterCopy( saturationPressurePlotOriginal,
                                                                 saturationPressurePlotCopy );
            }
        }

        plots.push_back( copy );
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicSummaryPlotBuilder::duplicateSummaryPlots( const std::vector<RimSummaryPlot*>& sourcePlots )
{
    std::vector<RimSummaryPlot*> plots;

    for ( auto plot : sourcePlots )
    {
        auto copy =
            dynamic_cast<RimSummaryPlot*>( plot->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

        if ( copy )
        {
            plots.push_back( copy );
        }
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot* RicSummaryPlotBuilder::createAndAppendMultiPlot( const std::vector<RimPlot*>& plots )
{
    RimProject*             project        = RimProject::current();
    RimMultiPlotCollection* plotCollection = project->mainPlotCollection()->multiPlotCollection();

    auto* plotWindow = new RimMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    appendPlotsToMultiPlot( plotWindow, plots );

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    if ( !plots.empty() )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plots[0], true );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow, true );
    }

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot*
    RicSummaryPlotBuilder::createAndAppendSummaryMultiPlot( const std::vector<caf::PdmObjectHandle*>& objects )
{
    RimProject*                    project        = RimProject::current();
    RimSummaryMultiPlotCollection* plotCollection = project->mainPlotCollection()->summaryMultiPlotCollection();

    auto* plotWindow = new RimSummaryMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( plotWindow );

    plotWindow->handleDroppedObjects( objects );

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    if ( plotWindow->summaryPlots().size() == 1 )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow->summaryPlots()[0] );
        RiuPlotMainWindowTools::setExpanded( plotWindow->summaryPlots()[0], true );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow );
        RiuPlotMainWindowTools::setExpanded( plotWindow, true );
    }

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::appendPlotsToMultiPlot( RimMultiPlot* multiPlot, const std::vector<RimPlot*>& plots )
{
    for ( auto plot : plots )
    {
        // Remove the current window controller, as this will be managed by the multi plot
        // This must be done before adding the plot to the multi plot to ensure that the viewer widget is recreated
        plot->revokeMdiWindowStatus();

        multiPlot->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->setShowWindow( true );

        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot*
    RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( const std::vector<RimSummaryCase*>&           cases,
                                                                   const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    if ( prefs->defaultSummaryPlotType() == RiaPreferencesSummary::DefaultSummaryPlotType::NONE ) return nullptr;

    if ( prefs->defaultSummaryPlotType() == RiaPreferencesSummary::DefaultSummaryPlotType::PLOT_TEMPLATES )
    {
        RimSummaryMultiPlot* plotToSelect = nullptr;
        for ( auto& filename : prefs->defaultSummaryPlotTemplates() )
        {
            plotToSelect = RicSummaryPlotTemplateTools::create( filename, cases, ensembles );
        }
        return plotToSelect;
    }

    if ( prefs->defaultSummaryCurvesTextFilter().trimmed().isEmpty() ) return nullptr;

    RimProject* project        = RimProject::current();
    auto*       plotCollection = project->mainPlotCollection()->summaryMultiPlotCollection();

    auto* plotWindow = new RimSummaryMultiPlot();
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( plotWindow );

    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->setAsPlotMdiWindow();
    plot->enableAutoPlotTitle( true );

    for ( auto sumCase : cases )
    {
        RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot( plot, sumCase );
    }

    for ( auto ensemble : ensembles )
    {
        RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( plot, ensemble );
    }

    plot->applyDefaultCurveAppearances();
    plot->loadDataAndUpdate();

    plotCollection->updateConnectedEditors();

    appendPlotsToSummaryMultiPlot( plotWindow, { plot } );

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();
    plotWindow->updateAllRequiredEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotBuilder::createAndAppendSummaryMultiPlot( const std::vector<RimSummaryPlot*>& plots )
{
    RimProject* project        = RimProject::current();
    auto*       plotCollection = project->mainPlotCollection()->summaryMultiPlotCollection();

    auto* summaryMultiPlot = new RimSummaryMultiPlot();
    summaryMultiPlot->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( summaryMultiPlot );

    appendPlotsToSummaryMultiPlot( summaryMultiPlot, plots );

    summaryMultiPlot->setDefaultRangeAggregationSteppingDimension();
    summaryMultiPlot->zoomAll();

    plotCollection->updateAllRequiredEditors();
    summaryMultiPlot->loadDataAndUpdate();
    summaryMultiPlot->updateAllRequiredEditors();

    if ( !plots.empty() )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plots[0] );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( summaryMultiPlot );
    }

    return summaryMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( RimSummaryPlot* plot )
{
    RimProject* project        = RimProject::current();
    auto*       plotCollection = project->mainPlotCollection()->summaryMultiPlotCollection();

    auto* plotWindow = new RimSummaryMultiPlot();
    plotWindow->setColumnCount( RiaDefines::ColumnCount::COLUMNS_1 );
    plotWindow->setRowCount( RiaDefines::RowCount::ROWS_1 );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( plotWindow );

    appendPlotsToSummaryMultiPlot( plotWindow, { plot } );

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();
    plotWindow->updateAllRequiredEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::appendPlotsToSummaryMultiPlot( RimSummaryMultiPlot*                multiPlot,
                                                           const std::vector<RimSummaryPlot*>& plots )
{
    for ( auto plot : plots )
    {
        plot->revokeMdiWindowStatus();

        multiPlot->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->setShowWindow( true );

        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotBuilder::createPlot( const std::vector<RimSummaryCurve*>& summaryCurves )
{
    auto* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );

    for ( auto& curve : summaryCurves )
    {
        plot->addCurveNoUpdate( curve );
    }

    plot->applyDefaultCurveAppearances();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotBuilder::createPlot( const std::set<RifEclipseSummaryAddress>&     addresses,
                                                   const std::vector<RimSummaryCase*>&           summaryCases,
                                                   const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    auto* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );

    appendCurvesToPlot( plot, addresses, summaryCases, ensembles );

    plot->applyDefaultCurveAppearances();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::appendCurvesToPlot( RimSummaryPlot*                               summaryPlot,
                                                const std::set<RifEclipseSummaryAddress>&     addresses,
                                                const std::vector<RimSummaryCase*>&           summaryCases,
                                                const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    for ( const auto& addr : addresses )
    {
        for ( const auto ensemble : ensembles )
        {
            auto curveSet = createCurveSet( ensemble, addr );
            summaryPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );
        }

        for ( const auto summaryCase : summaryCases )
        {
            auto curve = createCurve( summaryCase, addr );

            summaryPlot->addCurveNoUpdate( curve );
        }
    }
}
