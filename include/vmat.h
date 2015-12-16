#ifndef INC_S2DC_VMAT_DECOMPILER_H
#define INC_S2DC_VMAT_DECOMPILER_H

#ifdef _WIN32
#pragma once
#endif

/* VMAT_C File Format

   DATA Block
     4 bytes:  Offset to material name (usually the same as the file, minus the _c)
     4 bytes:  Offset to shader name
     4 bytes:  Offset to int params
       -> 4 bytes:  Offset to param string
       -> 4 bytes:  Int param value
     4 bytes:  Number of int params
     4 bytes:  Offset to float params
       -> 4 bytes:  Offset to param string
       -> 4 bytes:  Float param value
     4 bytes:  Number of float params
     4 bytes:  Offset to vector params
       -> 4 bytes:   Offset to param string
       -> 16 bytes:  Vector param value (vector4)
     4 bytes:  Number of vector params
     4 bytes:  Offset to texture params
       -> 8 bytes:  Offset to param string
       -> 8 bytes:  Resource ID (see RERL Block)
     4 bytes:  Number of texture params
     4 bytes:  Offset to dynamic params
       -> 4 bytes:  Offset to material param name
       -> 4 bytes:  Offset to compiled dynamic expression (unknown)
     4 bytes:  Number of dynamic params
     4 bytes:  Offset to dynamic texture params
       -> Unknown
     4 bytes:  Number of dynamic texture params
     4 bytes:  Offset to int attributes
       -> 4 bytes:  Offset to attribute string
       -> 4 bytes:  Int attribute value
     4 bytes:  Number of int attributes
     4 bytes:  Offset to float attributes
       -> 4 bytes:  Offset to attribute string
       -> 4 bytes:  Float attribute value
     4 bytes:  Number of float attributes
     4 bytes:  Offset to vector attributes
       -> 4 bytes(?):  Offset to attribute string
       -> 16 bytes(?):  Vector attribute value
     4 bytes:  Number of vector attributes
     4 bytes:  Offset to texture attributes
       -> 8 bytes(?):  Offset to attribute string
       -> 8 bytes(?):  Resource ID
     4 bytes:  Number of texture attributes
     4 bytes:  Offset to string attributes
       -> 4 bytes:  Offset to attribute string
       -> 4 bytes:  Offset to string attribute value
     4 bytes:  Number of string attributes
*/

#endif