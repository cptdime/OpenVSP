//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Mesh Geometry Class
//
//
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//
//******************************************************************************

#ifndef MESH_GEOM_H
#define MESH_GEOM_H


#include "Vec2d.h"
#include "Geom.h"
#include "Util.h"

class MeshInfo
{
public:
    MeshInfo()
    {
        m_NumOpenMeshesMerged = m_NumOpenMeshedDeleted = m_NumDegenerateTriDeleted = 0;
    }

    int m_NumOpenMeshesMerged;
    int m_NumOpenMeshedDeleted;
    int m_NumDegenerateTriDeleted;
};


class MeshGeom : public Geom
{

    int m_BigEndianFlag;

    vector< TTri* > m_NascartTriVec;
    vector< TNode* > m_NascartNodeVec;

    vector< TTri* > m_TecplotTriVec[30];
    vector< TNode* > m_TecplotNodeVec[30];


public:
    enum { SLICE_PLANAR, SLICE_AWAVE };

    //==== How Mesh Is Drawn and Saved ====//
    enum { INTERSECTION_MESH, MODEL_MESH };

    MeshGeom( Vehicle* vehicle_ptr );
    ~MeshGeom();

    //! MeshGeom's EncodeXml Implementation
    /**
       MeshGeom's EncodeXml Method does not write out each TTri's splitVec. So make sure that FlattenTMeshVec has been called on MeshGeom
      before calling EncodeXml.
    */
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string m_FileName;

    double m_TotalTheoArea;
    double m_TotalWetArea;
    double m_TotalTheoVol;
    double m_TotalWetVol;

    int m_MeshFlag;              // Do WaterTight Check and Quality Mesh
    int m_MassPropFlag;
    int m_MeshType;
    vec3d m_CenterOfGrav;

    double m_TotalMass;
    double m_TotalIxx;
    double m_TotalIyy;
    double m_TotalIzz;
    double m_TotalIxy;
    double m_TotalIxz;
    double m_TotalIyz;

    double m_MinTriDen;
    double m_MaxTriDen;
    vector < TTri* > m_MpTriVec;

    vector < TMesh* > m_TMeshVec;
    vector < TMesh* > m_SliceVec;

    TMesh* m_OneMesh;

    // Scale Transformation Matrix
    Matrix4d m_ScaleMatrix;
    Parm m_ScaleFromOrig;

    virtual void SetMeshType( int type )
    {
        m_MeshType = type;
    }
    virtual int  GetMeshType()
    {
        return m_MeshType;
    }

    virtual void load_hidden_surf();
    virtual void load_normals();
    virtual void UpdateBBox();
    virtual void UpdateDrawObj();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual int  GetNumXSecSurfs()
    {
        return 0;
    }
    virtual int  ReadSTL( const char* file_name );
    virtual int  ReadXSec( const char* file_name );
    virtual void AddTri( TMesh* tMesh, vec3d & p0, vec3d & p1, vec3d & p2 );
    virtual int  ReadNascart( const char* file_name );
    virtual int  ReadTriFile( const char* file_name );
    virtual float ReadBinFloat( FILE* fptr );
    virtual int   ReadBinInt  ( FILE* fptr );
    virtual void WriteStl( FILE* pov_file );

    virtual void BuildNascartMesh( int partOffset );
    virtual int  GetNumNascartPnts()
    {
        return m_NascartNodeVec.size();
    }
    virtual int  GetNumTecplotPnts( int m )
    {
        return m_TecplotNodeVec[m].size();
    }
    virtual int  GetNumNascartTris()
    {
        return m_NascartTriVec.size();
    }
    virtual int  GetNumTecplotTris( int m )
    {
        return m_TecplotTriVec[m].size();
    }
    virtual int  GetNumNascartParts()
    {
        return m_TMeshVec.size();
    }
    virtual void WriteNascartPnts( FILE* file_id );
    virtual void WriteCart3DPnts( FILE* file_id );
    virtual int  WriteGMshNodes( FILE* file_id, int node_offset );
    virtual int  WriteNascartTris( FILE* file_id, int offset );
    virtual int  WriteCart3DTris( FILE* file_id, int offset );
    virtual int  WriteGMshTris( FILE* file_id, int node_offset, int tri_offset );
    virtual int  WriteNascartParts( FILE* file_id, int offset );
    virtual int  WriteCart3DParts( FILE* file_id, int offset );
    virtual void WritePovRay( FILE* fid, int comp_num );
    virtual void WriteX3D( xmlNodePtr node );
    virtual void CheckDupOrAdd( TNode* node, vector< TNode* > & nodeVec );

    virtual void dump_xsec_file( int, FILE* ) {}
    virtual void Scale();

    //==== Intersection, Splitting and Trimming ====//
    virtual void IntersectTrim( int meshFlag, int halfFlag = 0 );
    virtual void SliceX( int numSlice );
    virtual void MassSliceX( int numSlice );
    virtual void AreaSlice( int style, int numSlices, double sliceAngle, double coneSections, vec3d norm, bool autoBounds,
                            double start = 0, double end = 0 );
    virtual vector<vec3d> TessTriangles( vector<vec3d> &tri );
    virtual vector<vec3d> TessTri( vec3d t1, vec3d t2, vec3d t3, int iterations );

    virtual void MergeRemoveOpenMeshes( MeshInfo* info );

    virtual vec3d GetVertex3d( int surf, double x, double p, int r );
    //virtual void  getVertexVec(vector< VertexID > *vertVec);

    virtual void CreatePrism( vector< TetraMassProp* >& tetraVec, TTri* tri, double len );
    virtual void AddPointMass( TetraMassProp* pm )
    {
        m_PointMassVec.push_back( pm );
    }
    vector< TetraMassProp* > m_PointMassVec;

    virtual void WaterTightCheck( FILE* fid );
    virtual void AddHalfBox();

    virtual void UpdateSurf() {}
    virtual int GetNumMainSurfs()
    {
        return 0;
    }
    virtual vector< TMesh* > CreateTMeshVec();
    virtual void FlattenTMeshVec();
    virtual void FlattenSliceVec();
    virtual Matrix4d GetTotalTransMat();
    virtual void TransformMeshVec( vector<TMesh*> & meshVec, Matrix4d & TransMat );

protected:
    virtual void ApplyScale(); // this is for intersectTrim

};

#endif