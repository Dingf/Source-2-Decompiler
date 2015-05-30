#ifndef INC_S2DC_VMDL_DECOMPILER_H
#define INC_S2DC_VMDL_DECOMPILER_H

#ifdef _WIN32
#pragma once
#endif

/* VMDL_C File Format

   DATA Block
     4 bytes:  Offset to model name
	 Inline:   Model info struct
	   -> 4 bytes:   Flags (Possibly the same as MDL flags?)
	   -> 12 bytes:  Hull min (vector3)
	   -> 12 bytes:  Hull max (vector3)
	   -> 12 bytes:  View min (vector3)
	   -> 12 bytes:  View max (vector3)
	   -> 4 bytes:   Mass (float)
	   -> 12 bytes:  Eye position (vector3)
	   -> 4 bytes:   Max eye deflection
	   -> 4 bytes:   Offset to surface property string
	   -> 4 bytes:   Offset to key value text
	 4 bytes:  Offset to external parts
	 4 bytes:  Number of external parts
	 4 bytes:  Offset to reference meshes
	 4 bytes:  Number of reference meshes
	 4 bytes:  Offset to mesh group masks
	 4 bytes:  Number of mesh group masks
	 4 bytes:  Offset to physics group masks
	 4 bytes:  Number of physics group masks
	 4 bytes:  Offset to LOD group masks
	 4 bytes:  Number of LOD group masks
	 4 bytes:  Offset to LOD group switch distances
	 4 bytes:  Number of LOD group switch distances
	 4 bytes:  Offset to physics data
	 4 bytes:  Number of physics data
	 4 bytes:  Offset to physics hitbox data
	 4 bytes:  Number of physics hitbox data
	 4 bytes:  Offset to animation groups
	 4 bytes:  Number of animation groups
	 4 bytes:  Offset to sequence groups
	 4 bytes:  Number of sequence groups
	 4 bytes:  Offset to mesh groups
	 4 bytes:  Number of mesh groups
	 4 bytes:  Offset to material groups
	 4 bytes:  Number of material groups
	 8 bytes:  
*/


#endif