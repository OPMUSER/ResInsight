/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include <string>
#include <tuple>
#include <vector>

#include "opm/io/eclipse/EclFile.hpp"

class RifRftSegmentData
{
public:
    RifRftSegmentData( int segnxt, int brno, int brnst, int brnen, int segNo );

    int segNext() const;
    int segBrno() const;
    int segBrnst() const;
    int segBrnen() const;
    int segNo() const;

private:
    int m_segNext;
    int m_segbrno;
    int m_brnst;
    int m_brnen;
    int m_segmentNo;
};

class RifRftSegment
{
public:
    void                           setSegmentData( std::vector<RifRftSegmentData> segmentData );
    std::vector<RifRftSegmentData> topology() const;

    void addResultNameAndSize( const Opm::EclIO::EclFile::EclEntry& resultNameAndSize );
    std::vector<Opm::EclIO::EclFile::EclEntry> resultNameAndSize() const;

    std::vector<int> branchIds() const;

    std::vector<size_t> indicesForBranchNumber( int branchNumber ) const;

private:
    std::vector<RifRftSegmentData>             m_topology;
    std::vector<Opm::EclIO::EclFile::EclEntry> m_resultNameAndSize;
};
