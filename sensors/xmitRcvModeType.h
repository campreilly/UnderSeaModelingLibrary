/**
 *  @file xmitRcvModeType.h
 *  Definition of the enum xmitRcvModeType.
 */

#pragma once

namespace usml {
namespace sensors {

enum xmitRcvMode
{
	NONE = 0,
    RECEIVER = 1,
	SOURCE = 2,
	BOTH = 3
};

typedef xmitRcvMode xmitRcvModeType;

} // end of namespace sensors
} // end of namespace usml
