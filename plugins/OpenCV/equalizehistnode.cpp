#include "equalizehistnode.h"

#include <QtConcurrent/QtConcurrentRun>

#include "opencvplugin.h"

#include <fugio/global_interface.h>
#include "fugio/context_interface.h"
#include <fugio/pin_interface.h>
#include <fugio/pin_control_interface.h>

#include <fugio/performance.h>

#include <fugio/image/uuid.h>

#if defined( OPENCV_SUPPORTED )
#include <opencv2/imgproc/imgproc.hpp>
#endif

EqualizeHistNode::EqualizeHistNode( QSharedPointer<fugio::NodeInterface> pNode )
	: NodeControlBase( pNode )
{
	mPinInputImage = pinInput( "Image" );

	mPinInputImage->registerPinInputType( PID_IMAGE );

	mValOutputImage = pinOutput<fugio::ImageInterface *>( "Image", mPinOutputImage, PID_IMAGE );
}

void EqualizeHistNode::inputsUpdated( qint64 pTimeStamp )
{
	NodeControlBase::inputsUpdated( pTimeStamp );

	if( !mPinInputImage->isConnected() )
	{
		return;
	}

	fugio::ImageInterface			*SrcImg = input<fugio::ImageInterface *>( mPinInputImage );

	if( !SrcImg || SrcImg->size().isEmpty() )
	{
		return;
	}

#if defined( OPENCV_SUPPORTED )
	if( false )
	{
		mNode->context()->futureSync( QtConcurrent::run( &EqualizeHistNode::conversion, this ) );
	}
	else
	{
		fugio::Performance	P( mNode, "inputsUpdated", pTimeStamp );

		conversion( this );
	}
#endif
}

void EqualizeHistNode::conversion( EqualizeHistNode *pNode )
{
#if defined( OPENCV_SUPPORTED )
	fugio::ImageInterface		*SrcImg = pNode->input<fugio::ImageInterface *>( pNode->mPinInputImage );

	cv::Mat						 MatSrc = OpenCVPlugin::image2mat( SrcImg );

	try
	{
		cv::equalizeHist( MatSrc, pNode->mMatImg );

		pNode->mNode->setStatus( fugio::NodeInterface::Initialised );
	}
	catch( cv::Exception e )
	{
		pNode->mNode->setStatus( fugio::NodeInterface::Error );
		pNode->mNode->setStatusMessage( e.err.c_str() );
	}
	catch( ... )
	{

	}

	OpenCVPlugin::mat2image( pNode->mMatImg, pNode->mValOutputImage );

	pNode->pinUpdated( pNode->mPinOutputImage );
#endif
}

