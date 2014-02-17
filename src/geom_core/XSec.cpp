//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "XSec.h"
#include "Geom.h"
#include "XSecSurf.h"
//#include "SuperEllipse.h"
#include "Parm.h"
#include "VehicleMgr.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include <float.h>
#include <stdio.h>

#include "Vehicle.h"

#include "eli/geom/curve/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_point_creator<double, 3, curve_tolerance_type> piecewise_point_creator;
typedef eli::geom::curve::piecewise_circle_creator<double, 3, curve_tolerance_type> piecewise_circle_creator;
typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_ellipse_creator;
typedef eli::geom::curve::piecewise_superellipse_creator<double, 3, curve_tolerance_type> piecewise_superellipse_creator;

using std::string;

//==== Default Constructor ====//
XSec::XSec( bool use_left )
{
    m_rotation.loadIdentity();
    m_center = false;

    m_UseLeftRef = use_left;
    m_GroupName = "XSec";
    m_RefLength = 1.0;
    m_XLocPercent.Init( "XLocPercent", m_GroupName, this,  0.0, 0.0, 1.0 );
    m_XLocPercent.SetDescript( "X distance of cross section as a percent of fuselage length" );
    m_YLocPercent.Init( "YLocPercent", m_GroupName, this,  0.0, -1.0, 1.0 );
    m_YLocPercent.SetDescript( "Y distance of cross section as a percent of fuselage length" );
    m_ZLocPercent.Init( "ZLocPercent", m_GroupName, this,  0.0, -1.0, 1.0 );
    m_ZLocPercent.SetDescript( "Z distance of cross section as a percent of fuselage length" );

    m_XRotate.Init( "XRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_XRotate.SetDescript( "Rotation about x-axis of cross section" );
    m_YRotate.Init( "YRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about y-axis of cross section" );
    m_ZRotate.Init( "ZRotate", m_GroupName, this,  0.0, -180.0, 180.0 );
    m_YRotate.SetDescript( "Rotation about z-axis of cross section" );

    m_Spin.Init( "Spin", m_GroupName, this, 0.0, -180.0, 180.0 );

    double skinningAngleMin( -90 ), skinningAngleMax( 90 );
    double skinningStrengthMin( 1e-5 ), skinningStrengthMax( 1e5 );
    double skinningCurvatureMin( -1e4 ), skinningCurvatureMax( 1e4 );

    m_ContinuityFlag.Init( "TopSideContinuityFlag", "Skinning", this, VspJointInfo::C0, 0, VspJointInfo::NUM_CONTINUITY_TYPES );
    m_ContinuityFlag.SetDescript( "Type of continuity to specify on this segment" );

    m_NRightSecs.Init( "NRightSecs", "Skinning", this, 6, 2, 100 );
    m_NRightSecs.SetDescript( "Number of sections to display on this segment" );

    m_TRBLSymmetryFlag.Init( "TRBLSymmetryFlag", "Skinning", this, true, false, true );
    m_TRBLSymmetryFlag.SetDescript( "Enforces top, bottom, left, and right sides all have same condition values." );
    m_TBSymmetryFlag.Init( "TBSymmetryFlag", "Skinning", this, true, false, true );
    m_TBSymmetryFlag.SetDescript( "Enforces top and bottom sides have same condition values." );
    m_RLSymmetryFlag.Init( "RLSymmetryFlag", "Skinning", this, true, false, true );
    m_RLSymmetryFlag.SetDescript( "Enforces left and right sides have same condition values." );

    m_LeftSegTopSideAngle.Init( "LeftSegTopSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_LeftSegTopSideAngle.SetDescript( "Tangent angle of cross section top connecting curve on left side of segment" );
    m_RightSegTopSideAngle.Init( "RightSegTopSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_RightSegTopSideAngle.SetDescript( "Tangent angle of cross section top connecting curve on right side of segment" );
    m_LeftSegTopSideStrength.Init( "LeftSegTopSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_LeftSegTopSideStrength.SetDescript( "Slope strength of cross section top connecting curve on left side of segment" );
    m_RightSegTopSideStrength.Init( "RightSegTopSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_RightSegTopSideStrength.SetDescript( "Slope strength of cross section top connecting curve on right side of segment" );
    m_LeftSegTopSideCurvature.Init( "LeftSegTopSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_LeftSegTopSideCurvature.SetDescript( "Curvature of cross section top connecting curve on left side of segment" );
    m_RightSegTopSideCurvature.Init( "RightSegTopSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_RightSegTopSideCurvature.SetDescript( "Curvature of cross section top connecting curve on right side of segment" );

    m_LeftSegRightSideAngle.Init( "LeftSegRightSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_LeftSegRightSideAngle.SetDescript( "Tangent angle of cross section right connecting curve on left side of segment" );
    m_RightSegRightSideAngle.Init( "RightSegRightSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_RightSegRightSideAngle.SetDescript( "Tangent angle of cross section right connecting curve on right side of segment" );
    m_LeftSegRightSideStrength.Init( "LeftSegRightSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_LeftSegRightSideStrength.SetDescript( "Slope strength of cross section right connecting curve on left side of segment" );
    m_RightSegRightSideStrength.Init( "RightSegRightSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_RightSegRightSideStrength.SetDescript( "Slope strength of cross section right connecting curve on right side of segment" );
    m_LeftSegRightSideCurvature.Init( "LeftSegRightSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_LeftSegRightSideCurvature.SetDescript( "Curvature of cross section right connecting curve on left side of segment" );
    m_RightSegRightSideCurvature.Init( "RightSegRightSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_RightSegRightSideCurvature.SetDescript( "Curvature of cross section right connecting curve on right side of segment" );

    m_LeftSegBottomSideAngle.Init( "LeftSegBottomSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_LeftSegBottomSideAngle.SetDescript( "Tangent angle of cross section bottom connecting curve on left side of segment" );
    m_RightSegBottomSideAngle.Init( "RightSegBottomSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_RightSegBottomSideAngle.SetDescript( "Tangent angle of cross section bottom connecting curve on right side of segment" );
    m_LeftSegBottomSideStrength.Init( "LeftSegBottomSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_LeftSegBottomSideStrength.SetDescript( "Slope strength of cross section bottom connecting curve on left side of segment" );
    m_RightSegBottomSideStrength.Init( "RightSegBottomSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_RightSegBottomSideStrength.SetDescript( "Slope strength of cross section bottom connecting curve on right side of segment" );
    m_LeftSegBottomSideCurvature.Init( "LeftSegBottomSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_LeftSegBottomSideCurvature.SetDescript( "Curvature of cross section bottom connecting curve on left side of segment" );
    m_RightSegBottomSideCurvature.Init( "RightSegBottomSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_RightSegBottomSideCurvature.SetDescript( "Curvature of cross section bottom connecting curve on right side of segment" );

    m_LeftSegLeftSideAngle.Init( "LeftSegLeftSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_LeftSegLeftSideAngle.SetDescript( "Tangent angle of cross section left connecting curve on left side of segment" );
    m_RightSegLeftSideAngle.Init( "RightSegLeftSideAngle", "Skinning", this, 0.0, skinningAngleMin, skinningAngleMax );
    m_RightSegLeftSideAngle.SetDescript( "Tangent angle of cross section left connecting curve on right side of segment" );
    m_LeftSegLeftSideStrength.Init( "LeftSegLeftSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_LeftSegLeftSideStrength.SetDescript( "Slope strength of cross section left connecting curve on left side of segment" );
    m_RightSegLeftSideStrength.Init( "RightSegLeftSideStrength", "Skinning", this, 0.5, skinningStrengthMin, skinningStrengthMax );
    m_RightSegLeftSideStrength.SetDescript( "Slope strength of cross section left connecting curve on right side of segment" );
    m_LeftSegLeftSideCurvature.Init( "LeftSegLeftSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_LeftSegLeftSideCurvature.SetDescript( "Curvature of cross section left connecting curve on left side of segment" );
    m_RightSegLeftSideCurvature.Init( "RightSegLeftSideCurvature", "Skinning", this, 0.0, skinningCurvatureMin, skinningCurvatureMax );
    m_RightSegLeftSideCurvature.SetDescript( "Curvature of cross section left connecting curve on right side of segment" );
}


void XSec::ChangeID( string newid )
{
    string oldid = m_ID;
    ParmContainer::ChangeID( newid );

    XSecSurf* xssurf = ( XSecSurf* ) GetParentContainerPtr();

    xssurf->ChangeXSecID( oldid, newid );
}

void XSec::SetGroupDisplaySuffix( int num )
{
    //==== Assign Group Suffix To All Parms ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

//==== Set Ref Length ====//
void XSec::SetRefLength( double len )
{
    if ( fabs( len - m_RefLength ) < DBL_EPSILON )
    {
        return;
    }

    m_RefLength = len;
    m_LateUpdateFlag = true;

    m_XLocPercent.SetRefVal( m_RefLength );
    m_YLocPercent.SetRefVal( m_RefLength );
    m_ZLocPercent.SetRefVal( m_RefLength );
}

//==== Set Scale ====//
void XSec::SetScale( double scale )
{
    SetWidthHeight( GetWidth()*scale, GetHeight()*scale );
    m_LeftSegTopSideCurvature.Set( m_LeftSegTopSideCurvature() / scale );
    m_RightSegTopSideCurvature.Set( m_RightSegTopSideCurvature() / scale );
    m_LeftSegRightSideCurvature.Set( m_LeftSegRightSideCurvature() / scale );
    m_RightSegRightSideCurvature.Set( m_RightSegRightSideCurvature() / scale );
    m_LeftSegBottomSideCurvature.Set( m_LeftSegBottomSideCurvature() / scale );
    m_RightSegBottomSideCurvature.Set( m_RightSegBottomSideCurvature() / scale );
    m_LeftSegLeftSideCurvature.Set( m_LeftSegLeftSideCurvature() / scale );
    m_RightSegLeftSideCurvature.Set( m_RightSegLeftSideCurvature() / scale );
}

//==== Parm Changed ====//
void XSec::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    // if number of sections changed let geom know
    if ( parm_ptr == &m_NRightSecs )
    {
        parm_ptr = 0;
        type = -1001;
    }

    Update();

    //==== Notify Parent Container (XSecSurf) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

//==== Update ====//
void XSec::Update()
{
    m_LateUpdateFlag = false;

    // apply the needed transformation to get section into body orientation
    Matrix4d mat( m_rotation );
    double *pm( mat.data() );

    pm[3] = 0;
    pm[7] = 0;
    pm[11] = 0;
    pm[12] = 0;
    pm[13] = 0;
    pm[14] = 0;
    pm[15] = 0;
    if ( m_center )
    {
        pm[13] = -GetWidth() / 2;
    }

    m_Curve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = m_Curve;
    if ( fabs( m_Spin() ) > DBL_EPSILON )
    {
        std::cerr << "XSec spin not implemented." << std::endl;
// NOTE: Not implementing spin. Also, this implementation doesn't spin first or last segments
//      double val = 0.0;
//      if ( m_Spin() > 0 )         val = 0.5*(m_Spin()/180.0);
//      else if ( m_Spin() < 0 )    val = 1.0 + 0.5*(m_Spin()/180.0);
//      val = Clamp( val, 0.0, 0.99999999 );
//
//      vector< vec3d > ctl_pnts = m_Curve.GetControlPnts();
//      int split = (int)(val*(((int)ctl_pnts.size()-1)/3)+1);
//
//      vector< vec3d > spin_ctl_pnts;
//      for ( int i = split*3 ; i < (int)ctl_pnts.size()-1 ; i++ )
//          spin_ctl_pnts.push_back( ctl_pnts[i] );
//
//      for ( int i = 0 ; i <= split*3 ; i++ )
//          spin_ctl_pnts.push_back( ctl_pnts[i] );
//
//      m_TransformedCurve.SetControlPnts( spin_ctl_pnts );
    }

    m_TransformedCurve.RotateX( m_XRotate()*DEG_2_RAD );
    m_TransformedCurve.RotateY( m_YRotate()*DEG_2_RAD );
    m_TransformedCurve.RotateZ( m_ZRotate()*DEG_2_RAD );

    m_TransformedCurve.OffsetX( m_XLocPercent()*m_RefLength );
    m_TransformedCurve.OffsetY( m_YLocPercent()*m_RefLength );
    m_TransformedCurve.OffsetZ( m_ZLocPercent()*m_RefLength );

    double right_value[3], left_value[3];
    int continuity;

    // set the top side values
    continuity = m_ContinuityFlag();
    right_value[0] = m_RightSegTopSideAngle();
    right_value[1] = m_RightSegTopSideStrength();
    right_value[2] = m_RightSegTopSideCurvature();
    left_value[0] = m_LeftSegTopSideAngle();
    left_value[1] = m_LeftSegTopSideStrength();
    left_value[2] = m_LeftSegTopSideCurvature();
    if ( m_UseLeftRef )
    {
        SetValues( continuity, right_value, left_value );
    }
    else
    {
        SetValues( continuity, left_value, right_value );
    }
    m_RightSegTopSideAngle.Set( right_value[0] );
    m_RightSegTopSideStrength.Set( right_value[1] );
    m_RightSegTopSideCurvature.Set( right_value[2] );
    m_LeftSegTopSideAngle.Set( left_value[0] );
    m_LeftSegTopSideStrength.Set( left_value[1] );
    m_LeftSegTopSideCurvature.Set( left_value[2] );

    if ( m_TRBLSymmetryFlag() )
    {
        // set the other three sides values to be the same as the top
        m_RightSegRightSideAngle.Set( right_value[0] );
        m_RightSegRightSideStrength.Set( right_value[1] );
        m_RightSegRightSideCurvature.Set( right_value[2] );
        m_LeftSegRightSideAngle.Set( left_value[0] );
        m_LeftSegRightSideStrength.Set( left_value[1] );
        m_LeftSegRightSideCurvature.Set( left_value[2] );

        m_RightSegBottomSideAngle.Set( right_value[0] );
        m_RightSegBottomSideStrength.Set( right_value[1] );
        m_RightSegBottomSideCurvature.Set( right_value[2] );
        m_LeftSegBottomSideAngle.Set( left_value[0] );
        m_LeftSegBottomSideStrength.Set( left_value[1] );
        m_LeftSegBottomSideCurvature.Set( left_value[2] );

        m_RightSegLeftSideAngle.Set( right_value[0] );
        m_RightSegLeftSideStrength.Set( right_value[1] );
        m_RightSegLeftSideCurvature.Set( right_value[2] );
        m_LeftSegLeftSideAngle.Set( left_value[0] );
        m_LeftSegLeftSideStrength.Set( left_value[1] );
        m_LeftSegLeftSideCurvature.Set( left_value[2] );
    }
    else
    {
        if ( !m_TBSymmetryFlag() )
        {
            // store the bottom side values
            right_value[0] = m_RightSegBottomSideAngle();
            right_value[1] = m_RightSegBottomSideStrength();
            right_value[2] = m_RightSegBottomSideCurvature();
            left_value[0] = m_LeftSegBottomSideAngle();
            left_value[1] = m_LeftSegBottomSideStrength();
            left_value[2] = m_LeftSegBottomSideCurvature();
            if ( m_UseLeftRef )
            {
                SetValues( continuity, right_value, left_value );
            }
            else
            {
                SetValues( continuity, left_value, right_value );
            }
        }
        m_RightSegBottomSideAngle.Set( right_value[0] );
        m_RightSegBottomSideStrength.Set( right_value[1] );
        m_RightSegBottomSideCurvature.Set( right_value[2] );
        m_LeftSegBottomSideAngle.Set( left_value[0] );
        m_LeftSegBottomSideStrength.Set( left_value[1] );
        m_LeftSegBottomSideCurvature.Set( left_value[2] );

        // set the right side values
        right_value[0] = m_RightSegRightSideAngle();
        right_value[1] = m_RightSegRightSideStrength();
        right_value[2] = m_RightSegRightSideCurvature();
        left_value[0] = m_LeftSegRightSideAngle();
        left_value[1] = m_LeftSegRightSideStrength();
        left_value[2] = m_LeftSegRightSideCurvature();
        if ( m_UseLeftRef )
        {
            SetValues( continuity, right_value, left_value );
        }
        else
        {
            SetValues( continuity, left_value, right_value );
        }
        m_RightSegRightSideAngle.Set( right_value[0] );
        m_RightSegRightSideStrength.Set( right_value[1] );
        m_RightSegRightSideCurvature.Set( right_value[2] );
        m_LeftSegRightSideAngle.Set( left_value[0] );
        m_LeftSegRightSideStrength.Set( left_value[1] );
        m_LeftSegRightSideCurvature.Set( left_value[2] );

        if ( !m_RLSymmetryFlag() )
        {
            // store the bottom side values
            right_value[0] = m_RightSegLeftSideAngle();
            right_value[1] = m_RightSegLeftSideStrength();
            right_value[2] = m_RightSegLeftSideCurvature();
            left_value[0] = m_LeftSegLeftSideAngle();
            left_value[1] = m_LeftSegLeftSideStrength();
            left_value[2] = m_LeftSegLeftSideCurvature();
            if ( m_UseLeftRef )
            {
                SetValues( continuity, right_value, left_value );
            }
            else
            {
                SetValues( continuity, left_value, right_value );
            }
            m_RightSegLeftSideAngle.Set( right_value[0] );
            m_RightSegLeftSideStrength.Set( right_value[1] );
            m_RightSegLeftSideCurvature.Set( right_value[2] );
            m_LeftSegLeftSideAngle.Set( left_value[0] );
            m_LeftSegLeftSideStrength.Set( left_value[1] );
            m_LeftSegLeftSideCurvature.Set( left_value[2] );
        }
    }
}

//==== Get Curve ====//
VspCurve& XSec::GetCurve()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_TransformedCurve;
}

//==== Get Untransformed Curve ====//
VspCurve& XSec::GetUntransformedCurve()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve;
}

VspJointInfo XSec::GetJointInfo()
{
    VspJointInfo joint;

    joint.SetState( m_ContinuityFlag() );

    joint.SetLeftParams( VspJointInfo::TOP_SIDE, DEG_2_RAD * m_LeftSegTopSideAngle(), m_LeftSegTopSideStrength(), m_LeftSegTopSideCurvature() );
    joint.SetLeftParams( VspJointInfo::RIGHT_SIDE, DEG_2_RAD * m_LeftSegRightSideAngle(), m_LeftSegRightSideStrength(), m_LeftSegRightSideCurvature() );
    joint.SetLeftParams( VspJointInfo::BOTTOM_SIDE, DEG_2_RAD * m_LeftSegBottomSideAngle(), m_LeftSegBottomSideStrength(), m_LeftSegBottomSideCurvature() );
    joint.SetLeftParams( VspJointInfo::LEFT_SIDE, DEG_2_RAD * m_LeftSegLeftSideAngle(), m_LeftSegLeftSideStrength(), m_LeftSegLeftSideCurvature() );
    joint.SetRightParams( VspJointInfo::TOP_SIDE, DEG_2_RAD * m_RightSegTopSideAngle(), m_RightSegTopSideStrength(), m_RightSegTopSideCurvature() );
    joint.SetRightParams( VspJointInfo::RIGHT_SIDE, DEG_2_RAD * m_RightSegRightSideAngle(), m_RightSegRightSideStrength(), m_RightSegRightSideCurvature() );
    joint.SetRightParams( VspJointInfo::BOTTOM_SIDE, DEG_2_RAD * m_RightSegBottomSideAngle(), m_RightSegBottomSideStrength(), m_RightSegBottomSideCurvature() );
    joint.SetRightParams( VspJointInfo::LEFT_SIDE, DEG_2_RAD * m_RightSegLeftSideAngle(), m_RightSegLeftSideStrength(), m_RightSegLeftSideCurvature() );

    return joint;
}

void XSec::SetJointInfo( const VspJointInfo &joint )
{
    m_ContinuityFlag.Set( joint.GetState() );

    m_LeftSegTopSideAngle.Set( RAD_2_DEG * joint.GetLeftAngle( VspJointInfo::TOP_SIDE ) );
    m_LeftSegTopSideStrength.Set( joint.GetLeftStrength( VspJointInfo::TOP_SIDE ) );
    m_LeftSegTopSideCurvature.Set( joint.GetLeftCurvature( VspJointInfo::TOP_SIDE ) );
    m_RightSegTopSideAngle.Set( RAD_2_DEG * joint.GetRightAngle( VspJointInfo::TOP_SIDE ) );
    m_RightSegTopSideStrength.Set( joint.GetRightStrength( VspJointInfo::TOP_SIDE ) );
    m_RightSegTopSideCurvature.Set( joint.GetRightCurvature( VspJointInfo::TOP_SIDE ) );

    m_LeftSegRightSideAngle.Set( RAD_2_DEG * joint.GetLeftAngle( VspJointInfo::RIGHT_SIDE ) );
    m_LeftSegRightSideStrength.Set( joint.GetLeftStrength( VspJointInfo::RIGHT_SIDE ) );
    m_LeftSegRightSideCurvature.Set( joint.GetLeftCurvature( VspJointInfo::RIGHT_SIDE ) );
    m_RightSegRightSideAngle.Set( RAD_2_DEG * joint.GetRightAngle( VspJointInfo::RIGHT_SIDE ) );
    m_RightSegRightSideStrength.Set( joint.GetRightStrength( VspJointInfo::RIGHT_SIDE ) );
    m_RightSegRightSideCurvature.Set( joint.GetRightCurvature( VspJointInfo::RIGHT_SIDE ) );

    m_LeftSegBottomSideAngle.Set( RAD_2_DEG * joint.GetLeftAngle( VspJointInfo::BOTTOM_SIDE ) );
    m_LeftSegBottomSideStrength.Set( joint.GetLeftStrength( VspJointInfo::BOTTOM_SIDE ) );
    m_LeftSegBottomSideCurvature.Set( joint.GetLeftCurvature( VspJointInfo::BOTTOM_SIDE ) );
    m_RightSegBottomSideAngle.Set( RAD_2_DEG * joint.GetRightAngle( VspJointInfo::BOTTOM_SIDE ) );
    m_RightSegBottomSideStrength.Set( joint.GetRightStrength( VspJointInfo::BOTTOM_SIDE ) );
    m_RightSegBottomSideCurvature.Set( joint.GetRightCurvature( VspJointInfo::BOTTOM_SIDE ) );

    m_LeftSegLeftSideAngle.Set( RAD_2_DEG * joint.GetLeftAngle( VspJointInfo::LEFT_SIDE ) );
    m_LeftSegLeftSideStrength.Set( joint.GetLeftStrength( VspJointInfo::LEFT_SIDE ) );
    m_LeftSegLeftSideCurvature.Set( joint.GetLeftCurvature( VspJointInfo::LEFT_SIDE ) );
    m_RightSegLeftSideAngle.Set( RAD_2_DEG * joint.GetRightAngle( VspJointInfo::LEFT_SIDE ) );
    m_RightSegLeftSideStrength.Set( joint.GetRightStrength( VspJointInfo::LEFT_SIDE ) );
    m_RightSegLeftSideCurvature.Set( joint.GetRightCurvature( VspJointInfo::LEFT_SIDE ) );
}

//==== Copy From XSec ====//
void XSec::CopyFrom( XSec* xs )
{
    string id = GetID();        // Save ID

    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );
    if ( xs->GetType() == GetType() )
    {
        xs->EncodeXml( root );
        DecodeXml( root );
    }
    else
    {
        int type = GetType();
        xs->XSec::EncodeXml( root );
        DecodeXml( root );
        m_Type = type;

        SetWidthHeight( xs->GetWidth(), xs->GetHeight() );
    }
    xmlFreeNode( root );
}

//==== Encode XML ====//
xmlNodePtr XSec::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSec", NULL );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
        XmlUtil::AddStringNode( xsec_node, "GroupName", m_GroupName );
        XmlUtil::AddStringNode( xsec_node, "ParentGeomID", m_ParentGeomID );
    }
    return xsec_node;
}

//==== Decode XML ====//
xmlNodePtr XSec::DecodeXml(  xmlNodePtr & node  )
{
    ParmContainer::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "XSec", 0 );
    if ( child_node )
    {
        m_GroupName = XmlUtil::FindString( child_node, "GroupName", m_GroupName );
        m_ParentGeomID = XmlUtil::FindString( child_node, "ParentGeomID", m_ParentGeomID );
    }
    return child_node;
}

//==== Encode XSec ====//
xmlNodePtr XSec::EncodeXSec(  xmlNodePtr & node  )
{
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSec", NULL );
    if ( xsec_node )
    {
        EncodeXml( xsec_node );
    }
    return xsec_node;
}

//==== Decode XSec ====//
xmlNodePtr XSec::DecodeXSec(  xmlNodePtr & node   )
{
    if ( node )
    {
        DecodeXml( node );
    }
    return node;
}

//==== Compute Area ====//
double XSec::ComputeArea( int num_pnts )
{
    VspCurve curve = GetCurve();
    vector<vec3d> pnts;
    curve.Tesselate( num_pnts, pnts );
    vec3d zero;
    return poly_area( pnts, zero );
}

void XSec::SetTransformation( const Matrix4d &mat, bool center )
{
    m_rotation = mat;
    m_center = center;
}


void XSec::SetValues( int continuity, double dep_values[3], const double ref_values[3] ) const
{
    switch ( continuity )
    {
    case( VspJointInfo::FULL ):
    case( VspJointInfo::C0 ):
    case( VspJointInfo::C1_AUTO ):
    {
        break;
    }
    case( VspJointInfo::G2 ):
    case( VspJointInfo::C2_AUTO ):
    case( VspJointInfo::C1 ):
    {
        dep_values[0] = ref_values[0];
        dep_values[1] = ref_values[1];
        break;
    }
    case( VspJointInfo::G1 ):
    {
        dep_values[0] = ref_values[0];
        break;
    }
    case( VspJointInfo::C2 ):
    {
        dep_values[0] = ref_values[0];
        dep_values[1] = ref_values[1];
        dep_values[2] = ref_values[2];
        break;
    }
    default:
    {
        std::cerr << "Invalid Continuity value of " <<  continuity << " at line " << __LINE__ << " of XSec" << std::endl;
        break;
    }
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
PointXSec::PointXSec( bool use_left ) : XSec( use_left )
{
    m_Type = POINT;
}

//==== Update Geometry ====//
void PointXSec::Update()
{
    piecewise_curve_type c;
    curve_point_type pt;
    pt << 0, 0, 0;

    // create point with 4 segments
    piecewise_point_creator ppc( 4 );

    // set point, make sure have 4 sections that go from 0 to 4
    ppc.set_point( pt );
    ppc.set_t0( 0 );
    ppc.set_segment_dt( 1, 0 );
    ppc.set_segment_dt( 1, 1 );
    ppc.set_segment_dt( 1, 2 );
    ppc.set_segment_dt( 1, 3 );
    if ( !ppc.create( c ) )
    {
        std::cerr << "Failed to create point XSec. " << __LINE__ << std::endl;
    }
    else
    {
        m_Curve.SetCurve( c );

        XSec::Update();
    }
}
//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
CircleXSec::CircleXSec( bool use_left ) : XSec( use_left )
{
    m_Type = CIRCLE;
    m_Diameter.Init( "Circle_Diameter", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of Circle Cross-Section" );
}

//==== Set Width and Height ====//
void CircleXSec::SetWidthHeight( double w, double h )
{
    m_Diameter  = ( w + h ) / 2.0;
}

//==== Update Geometry ====//
void CircleXSec::Update()
{
    piecewise_curve_type c;
    piecewise_circle_creator pcc( 4 );
    curve_point_type origin, start, normal;

    origin << m_Diameter() / 2, 0, 0;
    normal << 0, 0, -1;

    // set circle params, make sure that entire curve goes from 0 to 4
    pcc.set_origin( origin );
    pcc.set_radius( m_Diameter() / 2 );
    pcc.set_t0( 0 );
    pcc.set_segment_dt( 1, 0 );
    pcc.set_segment_dt( 1, 1 );
    pcc.set_segment_dt( 1, 2 );
    pcc.set_segment_dt( 1, 3 );
    if ( !pcc.create( c ) )
    {
        std::cerr << "Failed to create circle XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSec::Update();
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
EllipseXSec::EllipseXSec( bool use_left ) : XSec( use_left )
{
    m_Type = ELLIPSE;

    m_Height.Init( "Ellipse_Height", "EllipseXSec", this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Ellipse Cross-Section" );
    m_Width.Init( "Ellipse_Width", "EllipseXSec", this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Ellipse Cross-Section" );
}

//==== Update Geometry ====//
void EllipseXSec::Update()
{
    piecewise_curve_type c;
    piecewise_ellipse_creator pec( 4 );
    curve_point_type origin, normal;

    origin << m_Width() / 2, 0, 0;
    normal << 0, 0, -1;

    // set circle params, make sure that entire curve goes from 0 to 4
    pec.set_origin( origin );
    pec.set_x_axis_radius( m_Width() / 2 );
    pec.set_y_axis_radius( m_Height() / 2 );
    pec.set_t0( 0 );
    pec.set_segment_dt( 1, 0 );
    pec.set_segment_dt( 1, 1 );
    pec.set_segment_dt( 1, 2 );
    pec.set_segment_dt( 1, 3 );
    if ( !pec.create( c ) )
    {
        std::cerr << "Failed to create ellipse XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSec::Update();
    }
}

//==== Set Width and Height ====//
void EllipseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
SuperXSec::SuperXSec( bool use_left ) : XSec( use_left )
{
    m_Type = SUPER_ELLIPSE;

    m_Height.Init( "Super_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Super Ellipse Cross-Section" );
    m_Width.Init( "Super_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Super Ellipse Cross-Section" );
    m_M.Init( "Super_M", m_GroupName, this, 2.0, 0.2, 5.0 );
    m_M.SetDescript( "Width of the Super Ellipse Cross-Section" );
    m_N.Init( "Super_N", m_GroupName, this, 2.0, 0.2, 5.0 );
}

//==== Update Geometry ====//
void SuperXSec::Update()
{
    piecewise_curve_type c;
    piecewise_superellipse_creator psc( 16 );
    curve_point_type origin, x, y;

    origin << m_Width() / 2, 0, 0;

    // set hyperellipse params, make sure that entire curve goes from 0 to 4
    psc.set_axis( m_Width() / 2, m_Height() / 2 );
    psc.set_max_degree( 3 );
    psc.set_exponents( m_M(), m_N() );
    psc.set_origin( origin );

    psc.set_t0( 0 );
    for ( int i = 0; i < psc.get_number_segments(); ++i )
    {
        psc.set_segment_dt( 4.0 / psc.get_number_segments(), i );
    }

    if ( !psc.create( c ) )
    {
        std::cerr << "Failed to create superellipse XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSec::Update();
    }
}

//==== Set Width and Height ====//
void SuperXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
RoundedRectXSec::RoundedRectXSec( bool use_left ) : XSec( use_left )
{
    m_Type = ROUNDED_RECTANGLE;

    m_Height.Init( "RoundedRect_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "RoundedRect_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Radius.Init( "RoundRectXSec_Radius", m_GroupName,  this,  0.2, 0.0, 1.0e12 );
}

//==== Update Geometry ====//
void RoundedRectXSec::Update()
{
    VspCurve edge;
    vector<vec3d> pt;
    vector<double> u;
    double w = m_Width(), h = m_Height();
    double w2 = 0.5 * w, h2 = 0.5 * h, r;
    bool round_curve( true );

    // do some parameter checking
    if ( m_Radius() > w2 )
    {
        m_Radius.Set( w2 );
    }
    if ( m_Radius() > h2 )
    {
        m_Radius.Set( h2 );
    }
    r = m_Radius();

    // catch special cases of degenerate cases
    if ( ( w2 == 0 ) || ( h2 == 0 ) )
    {
        pt.resize( 4 );
        u.resize( 5 );

        pt[0].set_xyz( 0,   0,  h2 );
        pt[1].set_xyz( 0,  w2,   0 );
        pt[2].set_xyz( 0,   0, -h2 );
        pt[3].set_xyz( 0, -w2,   0 );

        u[0] = 0;
        u[1] = 1;
        u[2] = 2;
        u[3] = 3;
        u[4] = 4;

        round_curve = false;
    }
    // create rectangle
    else
    {
        pt.resize( 8 );
        u.resize( 9 );

        // set the segment points
        pt[0].set_xyz(  w,   0, 0 );
        pt[1].set_xyz(  w, -h2, 0 );
        pt[2].set_xyz( w2, -h2, 0 );
        pt[3].set_xyz(  0, -h2, 0 );
        pt[4].set_xyz(  0,   0, 0 );
        pt[5].set_xyz(  0,  h2, 0 );
        pt[6].set_xyz( w2,  h2, 0 );
        pt[7].set_xyz(  w,  h2, 0 );

        // set the corresponding parameters
        u[0] = 0;
        u[1] = h2 / ( h2 + w2 );
        u[2] = 1;
        u[3] = 1 + w2 / ( h2 + w2 );
        u[4] = 2;
        u[5] = 2 + h2 / ( h2 + w2 );
        u[6] = 3;
        u[7] = 3 + w2 / ( h2 + w2 );
        u[8] = 4;
    }

    // build the polygon
    m_Curve.InterpolateLinear( pt, u, true );

    // round all joints if needed
    if ( round_curve )
    {
        m_Curve.RoundAllJoints( r );
    }

    XSec::Update();
    return;
}

//==== Build Box of Even Spaced Points ====//
//==== Set Width and Height ====//
void RoundedRectXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
GeneralFuseXSec::GeneralFuseXSec( bool use_left ) : XSec( use_left )
{
    m_Type = GENERAL_FUSE;

    m_Height.Init( "Height",           m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "Width",             m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_MaxWidthLoc.Init( "MaxWidthLoc", m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
    m_CornerRad.Init( "CornerRad",     m_GroupName, this, 0.0, 0.0, 1.0e12 );
    m_TopTanAngle.Init( "TopTanAngle", m_GroupName, this, 90.0, 0.0, 90.0 );
    m_BotTanAngle.Init( "BotTanAngle", m_GroupName, this, 90.0, 0.0, 90.0 );
    m_TopStr.Init( "TopStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
    m_BotStr.Init( "BotStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
    m_UpStr.Init( "UpStr",             m_GroupName, this, 0.83, 0.0, 2.0 );
    m_LowStr.Init( "LowStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
}

//==== Set Width and Height ====//
void GeneralFuseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void GeneralFuseXSec::Update()
{
#if 0
    double y, z;
    //==== Top Control Points ====//
    vec3d tp0( 0.0, 0.0, m_Height() / 2.0 );
    vec3d tp1 = tp0 + vec3d( 0.0, m_TopStr() * m_Width() / 3.0, 0.0 );

    y = m_Width() / 2.0 + m_CornerRad() * m_Height() * ( sin( DEG_2_RAD * m_TopTanAngle() ) - 1.0 );
    z = m_MaxWidthLoc() * m_Height() / 2.0 + m_CornerRad() * m_Height() * cos( DEG_2_RAD * m_TopTanAngle() );
    vec3d tp3( 0.0, y, z );

    y = ( m_UpStr() * m_Height() * cos( DEG_2_RAD * m_TopTanAngle() ) ) / 3.0;
    z = ( -m_UpStr() * m_Height() * sin( DEG_2_RAD * m_TopTanAngle() ) ) / 3.0;
    vec3d tp2 = tp3 - vec3d( 0.0, y, z );

    //==== Bot Control Points ====//
    y = m_Width() / 2.0 + m_CornerRad() * m_Height() * ( sin( DEG_2_RAD * m_BotTanAngle() ) - 1.0 );
    z = m_MaxWidthLoc() * m_Height() / 2.0 - m_CornerRad() * m_Height() * cos( DEG_2_RAD * m_BotTanAngle() );
    vec3d bp0( 0.0, y, z );

    y = ( double )( -m_LowStr() * m_Height() * cos( DEG_2_RAD * m_BotTanAngle() ) ) / 3.0;
    z = ( double )( -m_LowStr() * m_Height() * sin( DEG_2_RAD * m_BotTanAngle() ) ) / 3.0;
    vec3d bp1 = bp0 + vec3d( 0.0, y, z );

    vec3d bp3( 0.0, 0.0, -m_Height() / 2.0 );
    vec3d bp2 = bp3 - vec3d( 0.0, -m_BotStr() * m_Width() / 3.0, 0.0 );

    //==== Corner Points ====//
    VspCurve corner_curve;
    bool corner_flag = true;
    if ( m_CornerRad() < 0.001 * m_Height() )
    {
        corner_flag = false;
    }

    double top_ang =  PI / 2.0 - DEG_2_RAD * m_TopTanAngle();
    double bot_ang = -PI / 2.0 + DEG_2_RAD * m_BotTanAngle();

    if ( ( top_ang - bot_ang ) < DBL_EPSILON )
    {
        corner_flag = false;
    }

    if ( corner_flag )
    {
        vector< vec3d > corner_pnts;
        int num_corner = 6;
        double del_ang =  ( 1.0 / ( double )( num_corner - 1 ) ) * ( top_ang - bot_ang );
        for ( int i = 0 ; i < num_corner  ; i++ )
        {
            double ang = top_ang - ( double )i * del_ang;
            y = ( double )( m_Width() / 2.0 + m_CornerRad() * m_Height() * ( cos( ang ) - 1.0 ) );
            z = ( double )( m_MaxWidthLoc() * m_Height() / 2.0 + m_CornerRad() * m_Height() * sin( ang ) );
            corner_pnts.push_back( vec3d( 0.0, y, z ) );

        }
        corner_curve.Interpolate( corner_pnts, false );
    }

    //==== Load Bezier Control Points ====//
    vector< vec3d > bez_pnts;
    bez_pnts.push_back( tp0 );
    bez_pnts.push_back( tp1 );
    bez_pnts.push_back( tp2 );
    bez_pnts.push_back( tp3 );

    if ( corner_flag )
    {
        vector< vec3d > corner_bez = corner_curve.GetControlPnts();
        for ( int i = 1 ; i < ( int )corner_bez.size() ; i++ )
        {
            bez_pnts.push_back( corner_bez[i] );
        }
    }

    bez_pnts.push_back( bp1 );
    bez_pnts.push_back( bp2 );
    bez_pnts.push_back( bp3 );

    //==== Reflect ====//
    int nrp = bez_pnts.size();
    for ( int i = 2 ; i <= nrp ; i++ )
    {
        vec3d p = bez_pnts[nrp - i];
        p.set_y( -p.y() );
        bez_pnts.push_back( p );
    }


    m_Curve.SetControlPnts( bez_pnts );
    m_Curve.UniformInterpolate( m_NumBasePnts, true );

    XSec::Update();
#endif
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FileXSec::FileXSec( bool use_left ) : XSec( use_left )
{
    m_Type = XSec::FILE_FUSE;

    m_UnityFilePnts.resize( 21, vec3d( 0, 0, 0 ) );

    m_Height.Init( "Height",           m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "Width",             m_GroupName, this, 1.0, 0.0, 1.0e12 );

}

//==== Set Width and Height ====//
void FileXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void FileXSec::Update()
{
#if 0
    //==== Scale File Points ====//
    vector< vec3d > scaled_file_pnts;
    for ( int i = 0 ; i < ( int )m_UnityFilePnts.size() ; i++ )
    {
        double y = m_UnityFilePnts[i].y() * m_Width();
        double z = m_UnityFilePnts[i].z() * m_Height();
        scaled_file_pnts.push_back( vec3d( 0.0, y, z ) );
    }

    m_Curve.Interpolate( scaled_file_pnts, true );
    m_Curve.UniformInterpolate( m_NumBasePnts, true );

    XSec::Update();
#endif
}

//==== Encode XML ====//
xmlNodePtr FileXSec::EncodeXml(  xmlNodePtr & node  )
{
    XSec::EncodeXml( node );
    xmlNodePtr child_node  = xmlNewChild( node, NULL, BAD_CAST "FileXSec", NULL );
    if ( child_node )
    {
        XmlUtil::AddVectorVec3dNode( child_node, "UnityFilePnts", m_UnityFilePnts );
    }
    return child_node;
}

//==== Decode XML ====//
xmlNodePtr FileXSec::DecodeXml(  xmlNodePtr & node  )
{
    XSec::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "FileXSec", 0 );
    if ( child_node )
    {
        m_UnityFilePnts = XmlUtil::ExtractVectorVec3dNode( child_node, "UnityFilePnts" );
    }
    return child_node;
}

//==== Read Fuse XSec File ====//
bool FileXSec::ReadXsecFile( string file_name )
{
    FILE* file_id =  fopen( file_name.c_str(), "r" );
    if ( file_id == ( FILE * )NULL )
    {
        return false;
    }

    m_FileName = file_name;

    if ( ReadOldXSecFile( file_id ) == true )
    {
        fclose( file_id );
        return true;
    }
    else if ( ReadXSecFile( file_id ) == true )
    {
        fclose( file_id );
        return true;
    }

    fclose( file_id );
    return false;
}

//==== Read Old Fuse XSec File ====//
bool FileXSec::ReadOldXSecFile( FILE* file_id )
{
    char buff[256];
    char name[256];
    string line;
    int num_pnts;

    rewind( file_id );
    fgets( buff, 255, file_id );
    line.assign( buff );

    //==== Check For Valid File Type ====//
    size_t found = line.find( "FUSE XSEC FILE" );
    if ( found == string::npos )
    {
        return false;
    }

    //==== Find XSec Name ====//
    fgets( buff, 80, file_id );
    sscanf( buff, "%s", name );
    m_FileName = string( name );

    //==== Find Num Pnts ====//
    fgets( buff, 80, file_id );
    sscanf( buff, "%d", &num_pnts );
    if ( num_pnts < 3 )
    {
        return false;
    }

    //==== Find Points ====//
    float y, z;
    vector< vec3d > pnt_vec;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        if ( fgets( buff, 80, file_id ) )
        {
            sscanf( buff, "%f  %f", &y, &z );
            pnt_vec.push_back( vec3d( 0.0, y, z ) );
        }
    }

    //==== Find Height & Width ====//
    vec3d p0 = pnt_vec[0];
    vec3d pn = pnt_vec[pnt_vec.size() - 1];
    m_Height.Set( max( fabs( p0.z() - pn.z() ), 1.0e-12 ) );

    double max_y = 0;
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double y = pnt_vec[i].y();
        if  ( fabs( y ) > max_y )
        {
            max_y = fabs( y );
        }
    }
    m_Width.Set( max( 2.0 * max_y, 1.0e-12 ) );

    //==== Scale Point By Height & Width ====//
    m_UnityFilePnts.clear();
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double y = pnt_vec[i].y() / m_Width();
        double z = pnt_vec[i].z() / m_Height();
        m_UnityFilePnts.push_back( vec3d( 0.0, y,  z ) );
    }
    //==== Reflected Pnts ====//
    for ( int i = ( int )pnt_vec.size() - 2 ; i >= 0 ; i-- )
    {
        double y = pnt_vec[i].y() / m_Width();
        double z = pnt_vec[i].z() / m_Height();
        m_UnityFilePnts.push_back( vec3d( 0.0, -y,  z ) );
    }
    return true;
}



//==== Read OpenVSP Fuse XSec File ====//
bool FileXSec::ReadXSecFile( FILE* file_id )
{
    char buff[256];
    string line;

    rewind( file_id );
    fgets( buff, 255, file_id );
    line.assign( buff );

    //==== Check For Valid File Type ====//
    size_t found = line.find( "OPENVSP_XSEC_FILE_V1" );
    if ( found == string::npos )
    {
        return false;
    }

    //==== Read Points ====//
    float y, z;
    vector< vec3d > pnt_vec;
    bool more_data = true;
    while( more_data )
    {
        more_data = false;
        if ( fgets( buff, 255, file_id ) )
        {
            sscanf( buff, "%f %f", &y, &z );
            if ( fabs( y ) < 1.0e12 && fabs( y ) <  1.0e12 )
            {
                pnt_vec.push_back( vec3d( 0.0, y, z ) );
                more_data = true;
            }
        }
    }

    SetPnts( pnt_vec );

    //int num_pnts = (int)pnt_vec.size();

    ////==== Find Height & Width ====//
    //m_Width  = 1.0e-12;
    //m_Height = 1.0e-12;
    //for ( int i = 0 ; i < num_pnts ; i++ )
    //{
    //  for ( int j = 0 ; j < num_pnts ; j++ )
    //  {
    //      double w = fabs( pnt_vec[i].y() - pnt_vec[j].y() );
    //      if ( w > m_Width() )    m_Width = w;
    //      double h = fabs( pnt_vec[i].z() - pnt_vec[j].z() );
    //      if ( h > m_Height() )   m_Height = h;
    //  }
    //}

    ////==== Scale Point By Height & Width ====//
    //m_UnityFilePnts.clear();
    //for ( int i = 0 ; i < (int)pnt_vec.size() ; i++ )
    //{
    //  double y = pnt_vec[i].y()/m_Width();
    //  double z = pnt_vec[i].z()/m_Height();
    //  m_UnityFilePnts.push_back( vec3d( 0.0, y,  z ) );
    //}

    return true;
}

//==== Set Pnt Vec ====//
void FileXSec::SetPnts( vector< vec3d > & pnt_vec )
{
    int num_pnts = ( int )pnt_vec.size();

    //==== Find Height & Width ====//
    m_Width  = 1.0e-12;
    m_Height = 1.0e-12;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            double w = fabs( pnt_vec[i].y() - pnt_vec[j].y() );
            if ( w > m_Width() )
            {
                m_Width = w;
            }
            double h = fabs( pnt_vec[i].z() - pnt_vec[j].z() );
            if ( h > m_Height() )
            {
                m_Height = h;
            }
        }
    }

    //==== Scale Point By Height & Width ====//
    m_UnityFilePnts.clear();
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double y = pnt_vec[i].y() / m_Width();
        double z = pnt_vec[i].z() / m_Height();
        m_UnityFilePnts.push_back( vec3d( 0.0, y,  z ) );
    }
}