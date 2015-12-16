#ifndef INC_S2DC_VPCF_DECOMPILER_H
#define INC_S2DC_VPCF_DECOMPILER_H

#ifdef _WIN32
#pragma once
#endif

/* VPCF_C File Format

   DATA Block
     4 bytes:   ID (always 0xAAA29DC3)
     4 bytes:   Number of initial particles
     12 bytes:  Bounding box min (vector3)
     12 bytes:  Bounding box max (vector3)
     4 bytes:   Snapshot control point
     4 bytes:   Offset to snapshot name
     4 bytes:   Offset to target layer ID
     8 bytes:   Replacement reference ID
     4 bytes:   Offset to cull replacement definition name
     4 bytes:   Cull radius (float)
     4 bytes:   Cull fill cost (float)
     4 bytes:   Cull control point
     8 bytes:   Max recreation time (padded float?)
     8 bytes:   Fallback reference ID
     8 bytes:   Fallback max count
     8 bytes:   Low violence reference ID
     4 bytes:   Color
     4 bytes:   Normal (float x3)
     4 bytes:   Radius (float)
     4 bytes:   Rotation (float)
     4 bytes:   Rotation speed (float)
     4 bytes:   Sequence number
     4 bytes:   Sequence number 1
     4 bytes:   Group ID
     4 bytes:   Maximum time step (float)
     4 bytes:   Maximum simulation tick rate (float)
     4 bytes:   Minimum simulation tick rate (float)
     4 bytes:   Minimum time step (float)
     4 bytes:   Minimum rendered frames
     4 bytes:   Minimum CPU level
     4 bytes:   Minimum GPU level
     4 bytes:   Flags (unconfirmed, non-zero = true)
       -> 1 byte:  Unknown
       -> 1 byte:  View model effect
       -> 1 byte:  Screen space effect
       -> 1 byte:  Unknown
     4 bytes:   Maximum draw distance (float)
     4 bytes:   Start fade distance (float)
     4 bytes:   Time to sleep when not drawn (float)
     4 bytes:   Maximum number of particles
     4 bytes:   CP to skip rendering if it is the camera
     4 bytes:   CP to only enable rendering if it is the camera
     4 bytes:   Minimum number of free particles to aggregate
     4 bytes:   Aggregation radius
     4 bytes:   Simulation stop time (float)
     12 bytes:  Default CP1 offset to CP0 (vector3)
     4 bytes:   Offset to particle name
     4 bytes:   Offset to children particles
       -> 8 bytes:  Child particle reference ID
       -> 4 bytes:  Flags
         --> 1 byte:   Prevent name-based lookup
         --> 3 bytes:  Unknown
       -> 4 bytes:  Delay (float)
       -> 4 btyes:  Flags
         --> 1 byte:   End cap
         --> 1 byte:   Disable child
         --> 2 bytes:  Unknown (padding?)
       -> 12 bytes  Unknown
     4 bytes:   Number of children particles
     4 bytes:   Flags
       -> 1 byte:   Sort particles flag
       -> 1 byte:   Batch particle systems flag
       -> 2 bytes:  Unknown (padding?)
     4 bytes:   Depth sort bias (4 bytes)
     4 bytes:   Flags
       -> 1 byte:   Hitbox fallback to render bounds flag
       -> 3 bytes:  Unknown
     4 bytes:   Offset to operators list
     4 bytes:   Number of operators
     4 bytes:   Offset to renderers list
     4 bytes:   Number of renderers
     4 bytes:   Offset to initializers list
     4 bytes:   Number of initializers
     4 bytes:   Offset to emitters list
     4 bytes:   Number of emitters
     4 bytes:   Offset to force generators list
     4 bytes:   Number of force generators
     4 bytes:   Offset to constraints list
     4 bytes:   Number of constraints
*/

#endif