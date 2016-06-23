#include "decodernode.h"

#include <QtEndian>
#include <QColor>

#include <fugio/core/uuid.h>
#include <fugio/core/list_interface.h>
#include <fugio/osc/split_interface.h>
#include <fugio/colour/colour_interface.h>

DecoderNode::DecoderNode( QSharedPointer<fugio::NodeInterface> pNode )
	: NodeControlBase( pNode )
{
	FUGID( PIN_INPUT_DATA,			"90AABEA7-AF08-49E6-888B-1EB41BA2F2C0" );
	FUGID( PIN_OUTPUT_NAMESPACE,	"6D57F958-E0E2-432D-A6B0-4A16812F500B" );

	mPinInput = pinInput( "Input", PIN_INPUT_DATA );

	mPinInput->registerPinInputTypes( QList<QUuid>{ PID_BYTEARRAY_LIST } );

	mValOutputNamespace = pinOutput<NamespacePin *>( "Namespace", mPinOutputNamespace, PID_OSC_NAMESPACE, PIN_OUTPUT_NAMESPACE );
}

void DecoderNode::inputsUpdated( qint64 pTimeStamp )
{
	Q_UNUSED( pTimeStamp )

	fugio::ListInterface	*LI = input<fugio::ListInterface *>( mPinInput );

	if( !LI )
	{
		return;
	}

	for( int i = 0 ; i < LI->listSize() ; i++ )
	{
		if( !LI->listIndex( i ).canConvert( QVariant::ByteArray ) )
		{
			continue;
		}

		processByteArray( LI->listIndex( i ).toByteArray() );
	}

	for( QString OscNam : mDataInput.keys() )
	{
		mValOutputNamespace->insert( OscNam, mDataInput.value( OscNam ).first() );

		QSharedPointer<fugio::PinInterface>		CurPin;

		QStringList		CurLst = OscNam.split( '/', QString::SkipEmptyParts );
		QStringList		NewLst;

		while( !CurPin && !CurLst.isEmpty() )
		{
			QString		CurNam = CurLst.join( '/' );

			CurPin = mNode->findOutputPinByName( CurNam );

			if( !CurPin )
			{
				NewLst.push_front( CurLst.takeLast() );
			}
		}

		if( !CurPin )
		{
			continue;
		}

		if( true )
		{
			fugio::osc::SplitInterface	*II = qobject_cast<fugio::osc::SplitInterface *>( CurPin->hasControl() ? CurPin->control()->qobject() : nullptr );

			if( II )
			{
				II->oscSplit( NewLst, mDataInput.value( OscNam ) );

				continue;
			}
		}

		if( true )
		{
			fugio::VariantInterface	*VI = qobject_cast<fugio::VariantInterface *>( CurPin->hasControl() ? CurPin->control()->qobject() : nullptr );

			if( VI )
			{
				VI->setVariant( mDataInput.value( OscNam ) );

				pinUpdated( CurPin );

				continue;
			}
		}
	}

	mDataInput.clear();
}

void DecoderNode::processByteArray( const QByteArray pByteArray )
{
	if( pByteArray.startsWith( "#bundle" ) )
	{
		processBundle( pByteArray );
	}
	else
	{
		processDatagram( pByteArray );
	}
}

void DecoderNode::processDatagram( const QByteArray &pDatagram )
{
	QByteArray		OscAdr;
	int				OscStr = 0;
	int				OscEnd = 0;

	OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
	OscAdr = pDatagram.mid( OscStr, OscEnd - OscStr );
	OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

	if( OscAdr.at( 0 ) != '/' )
	{
		return;
	}

	//OscAdr.remove( 0, 1 );

	QByteArray		OscArg;

	OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
	OscArg = pDatagram.mid( OscStr, OscEnd - OscStr );
	OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

	if( OscArg.at( 0 ) != ',' )
	{
		return;
	}

	OscArg.remove( 0, 1 );

	QVariantList	OscLst;

	for( int a = 0 ; a < OscArg.size() ; a++ )
	{
		int		OscInc = 0;

		switch( OscArg.at( a ) )
		{
			case 'i':	// int32
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					OscLst.append( v );

					OscInc = sizeof( v );
				}
				break;

			case 'f':	// float32
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					float		*f = (float *)&v;

					OscLst.append( *f );

					OscInc = sizeof( v );
				}
				break;

			case 's':	// OSC-string
				{
					QByteArray		OscVar;

					OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
					OscVar = pDatagram.mid( OscStr, OscEnd - OscStr );
					OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

					OscLst.append( QString( OscVar ) );
				}
				break;

			case 'b':	// OSC-blob
				break;

			case 'h':	// 64 bit big-endian two's complement integer
				{
					qlonglong		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					OscLst.append( v );

					OscInc = sizeof( v );
				}
				break;

			case 't':	// OSC-timetag
				break;

			case 'd':	// 64 bit ("double") IEEE 754 floating point number
				{
					qint64		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					double		*f = (double *)&v;

					OscLst.append( *f );

					OscInc = sizeof( v );
				}
				break;

			case 'S':	// Alternate type represented as an OSC-string (for example, for systems that differentiate "symbols" from "strings")
				break;

			case 'c':	// an ascii character, sent as 32 bits
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					OscLst.append( QChar( v ) );

					OscInc = sizeof( v );
				}
				break;

			case 'r':	// 32 bit RGBA color
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					const quint8 *p = (const quint8 *)&v;

					OscLst.append( QColor( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ] ) );

					OscInc = sizeof( v );
				}
				break;

			case 'm':	// 4 byte MIDI message. Bytes from MSB to LSB are: port id, status byte, data1, data2
				break;

			case 'T':	// True. No bytes are allocated in the argument data.
				OscLst.append( true );
				break;

			case 'F':	// False. No bytes are allocated in the argument data.
				OscLst.append( false );
				break;

			case 'N':	// Nil. No bytes are allocated in the argument data.
				break;

			case 'I':	// Infinitum. No bytes are allocated in the argument data.
				break;

			case '[':	// Indicates the beginning of an array. The tags following are for data in the Array until a close brace tag is reached.
				break;

			case ']':	// Indicates the end of an array.
				break;

			default:
				break;
		}

		OscStr += OscInc;
		OscEnd += OscInc;
	}

	if( OscLst.size() == 1 )
	{
		QHash<QString,QVariantList>::iterator	it = mDataInput.find( OscAdr );

		if( it != mDataInput.end() )
		{
			it.value().append( OscLst.first() );
		}
		else
		{
			QVariantList		VarLst;

			VarLst.append( OscLst.first() );

			mDataInput.insert( OscAdr, VarLst );
		}
	}
	else if( OscLst.size() > 1 )
	{
		QHash<QString,QVariantList>::iterator	it = mDataInput.find( OscAdr );

		if( it != mDataInput.end() )
		{
			it.value().append( QVariant( OscLst ) );
		}
		else
		{
			QVariantList		VarLst;

			VarLst.append( QVariant( OscLst ) );

			mDataInput.insert( OscAdr, VarLst );
		}
	}
}

void DecoderNode::processBundle( const QByteArray &pDatagram )
{
	QByteArray		OscAdr;
	int				OscStr = 0;
	int				OscEnd = 0;

	OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
	OscAdr = pDatagram.mid( OscStr, OscEnd - OscStr );
	OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

	quint64		TimeTag;

	memcpy( &TimeTag, pDatagram.data() + OscStr, 8 );

	OscStr += 8;

	TimeTag = qFromBigEndian( TimeTag );

	while( OscStr < pDatagram.size() )
	{
		quint32		ElementSize;

		memcpy( &ElementSize, pDatagram.data() + OscStr, 4 );

		ElementSize = qFromBigEndian( ElementSize );

		OscStr += 4;

		processDatagram( pDatagram.mid( OscStr, ElementSize ) );

		OscStr += ElementSize;
	}
}

QList<fugio::NodeControlInterface::AvailablePinEntry> DecoderNode::availableOutputPins() const
{
	return( mValOutputNamespace->oscPins( QStringList() ) );
}

QList<QUuid> DecoderNode::pinAddTypesOutput() const
{
	static QList<QUuid>	PinLst =
	{
		PID_BOOL,
		PID_COLOUR,
		PID_STRING,
		PID_FLOAT,
		PID_INTEGER,
		PID_BYTEARRAY,
		PID_OSC_SPLIT,
		PID_LIST
	};

	return( PinLst );
}

bool DecoderNode::canAcceptPin( fugio::PinInterface *pPin ) const
{
	return( pPin->direction() == PIN_INPUT );
}

QStringList DecoderNode::oscNamespace()
{
	return( mValOutputNamespace->oscNamespace() );
}

QList<fugio::NodeControlInterface::AvailablePinEntry> DecoderNode::oscPins(const QStringList &pCurDir) const
{
	return( mValOutputNamespace->oscPins( pCurDir ) );
}
