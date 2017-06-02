#include "mathplugin.h"

#include <QTranslator>
#include <QApplication>

#include <fugio/global_interface.h>
#include <fugio/global_signals.h>

#include <fugio/nodecontrolbase.h>

#include <fugio/math/uuid.h>

#include "matrix4pin.h"
#include "vector3pin.h"
#include "vector4pin.h"

#include "addnode.h"
#include "subtractnode.h"
#include "multiplynode.h"
#include "dividenode.h"

#include "absnode.h"

#include "vector3node.h"

#include "splitvector3.h"
#include "joinvector3.h"
#include "splitvector4node.h"
#include "joinvector4node.h"

#include "dotproductnode.h"
#include "crossproducenode.h"
#include "normalisenode.h"

#include "matrixlookatnode.h"
#include "matrixperspectivenode.h"
#include "matrixorthographicnode.h"

#include "matrixrotatenode.h"
#include "matrixtranslatenode.h"
#include "matrixscalenode.h"

#include "matrixmultiplynode.h"
#include "matrixinversenode.h"

#include "sindegreenode.h"
#include "sinradiannode.h"

#include "pinode.h"

#include "andnode.h"
#include "notnode.h"
#include "ornode.h"
#include "xornode.h"
#include "nandnode.h"
#include "nornode.h"
#include "xnornode.h"

#include "arccosnode.h"

#include "radianstodegreesnode.h"

#include "comparenumbersnode.h"

#include "rotationfromvectorsnode.h"

#include "minmaxnode.h"

#include "ceilnode.h"
#include "floornode.h"
#include "roundnode.h"

#include "quaternionpin.h"

#include "mathexpressionnode.h"

MathPlugin *MathPlugin::mInstance = nullptr;

QList<QUuid>	NodeControlBase::PID_UUID;

ClassEntry	NodeClasses[] =
{
	ClassEntry( "Abs", "Number", NID_ABS, &AbsNode::staticMetaObject ),
	ClassEntry( "Add", "Math", NID_ADD, &AddNode::staticMetaObject ),
	ClassEntry( "AND", "Logic", NID_AND, &AndNode::staticMetaObject ),
	ClassEntry( "ArcCos", "Number", NID_ARCCOS, &ArcCosNode::staticMetaObject ),
	ClassEntry( "Compare", "Number", NID_COMPARE_NUMBERS, &CompareNumbersNode::staticMetaObject ),
	ClassEntry( "Cross Product", "Vector3", NID_CROSS_PRODUCT, &CrossProductNode::staticMetaObject ),
	ClassEntry( "Radians to Degrees", "Number", NID_RADIANS_TO_DEGREES, &RadiansToDegreesNode::staticMetaObject ),
	ClassEntry( "Dot Product", "Vector3", NID_DOT_PRODUCT, &DotProductNode::staticMetaObject ),
	ClassEntry( "Ceil", "Math", NID_CEIL, &CeilNode::staticMetaObject ),
	ClassEntry( "Divide", "Math", NID_DIVIDE, &DivideNode::staticMetaObject ),
	ClassEntry( "Expression", "Math", NID_MATH_EXPRESSION, &MathExpressionNode::staticMetaObject ),
	ClassEntry( "Floor", "Math", NID_FLOOR, &FloorNode::staticMetaObject ),
	ClassEntry( "Inverse", "Matrix", NID_MATRIX_INVERSE, &MatrixInverseNode::staticMetaObject ),
	ClassEntry( "Join", "Vector3", NID_JOIN_VECTOR3, &JoinVector3Node::staticMetaObject ),
	ClassEntry( "Join", "Vector4", NID_JOIN_VECTOR4, &JoinVector4Node::staticMetaObject ),
	ClassEntry( "Look At", "Matrix", NID_MATRIX_LOOKAT, &MatrixLookAtNode::staticMetaObject ),
	ClassEntry( "MinMax", "Math", NID_MINMAX, &MinMaxNode::staticMetaObject ),
	ClassEntry( "Multiply", "Math", NID_MULTIPLY, &MultiplyNode::staticMetaObject ),
	ClassEntry( "NAND", "Logic", NID_NAND, &NandNode::staticMetaObject ),
	ClassEntry( "NOT", "Logic", NID_NOT, &NotNode::staticMetaObject ),
	ClassEntry( "NOR", "Logic", NID_NOR, &NorNode::staticMetaObject ),
	ClassEntry( "Normalise", "Vector3", NID_NORMALISE, &NormaliseNode::staticMetaObject ),
	ClassEntry( "OR", "Logic", NID_OR, &OrNode::staticMetaObject ),
	ClassEntry( "Orthographic", "Matrix", NID_MATRIX_ORTHOGRAPHIC, &MatrixOrthographicNode::staticMetaObject ),
	ClassEntry( "Perspective", "Matrix", NID_MATRIX_PERSPECTIVE, &MatrixPerspectiveNode::staticMetaObject ),
	ClassEntry( "Pi", "Number", NID_PI, &PiNode::staticMetaObject ),
	ClassEntry( "Rotate", "Matrix", NID_MATRIX_ROTATE, &MatrixRotateNode::staticMetaObject ),
	ClassEntry( "Rotation From Vectors", "Matrix", NID_ROTATION_FROM_VECTORS, &RotationFromVectorsNode::staticMetaObject ),
	ClassEntry( "Round", "Math", NID_ROUND, &RoundNode::staticMetaObject ),
	ClassEntry( "Scale", "Matrix", NID_MATRIX_SCALE, &MatrixScaleNode::staticMetaObject ),
	ClassEntry( "Sine (Degrees)", NID_SIN_DEGREES, &SinDegreeNode::staticMetaObject ),
	ClassEntry( "Sine (Radians)", NID_SIN_RADIANS, &SinRadianNode::staticMetaObject ),
	ClassEntry( "Split", "Vector3", NID_SPLIT_VECTOR3, &SplitVector3Node::staticMetaObject ),
	ClassEntry( "Split", "Vector4", NID_SPLIT_VECTOR4, &SplitVector4Node::staticMetaObject ),
	ClassEntry( "Subtract", "Math", NID_SUBTRACT, &SubtractNode::staticMetaObject ),
	ClassEntry( "Translate", "Matrix", NID_MATRIX_TRANSLATE, &MatrixTranslateNode::staticMetaObject ),
	ClassEntry( "Vector3", "GUI", NID_VECTOR3, &Vector3Node::staticMetaObject ),
	ClassEntry( "XNOR", "Logic", NID_XNOR, &XnorNode::staticMetaObject ),
	ClassEntry( "XOR", "Logic", NID_XOR, &XorNode::staticMetaObject ),
	ClassEntry()
};

ClassEntry PinClasses[] =
{
	ClassEntry( "Matrix4", PID_MATRIX4, &Matrix4Pin::staticMetaObject ),
	ClassEntry( "Quaternion", PID_QUATERNION, &QuaternionPin::staticMetaObject ),
	ClassEntry( "Vector3", PID_VECTOR3, &Vector3Pin::staticMetaObject ),
	ClassEntry( "Vector4", PID_VECTOR4, &Vector4Pin::staticMetaObject ),
	ClassEntry()
};

MathPlugin::MathPlugin() : mApp( 0 )
{
	mInstance = this;

	//-------------------------------------------------------------------------
	// Install translator

	static QTranslator		Translator;

	if( Translator.load( QLocale(), QLatin1String( "fugio_math" ), QLatin1String( "_" ), ":/translations" ) )
	{
		qApp->installTranslator( &Translator );
	}
}

PluginInterface::InitResult MathPlugin::initialise( fugio::GlobalInterface *pApp, bool pLastChance )
{
	Q_UNUSED( pLastChance )

	mApp = pApp;

	mApp->registerInterface( IID_MATH, this );

	mApp->registerNodeClasses( NodeClasses );

	mApp->registerPinClasses( PinClasses );

	mApp->registerPinSplitter( PID_VECTOR3, NID_SPLIT_VECTOR3 );
	mApp->registerPinSplitter( PID_VECTOR4, NID_SPLIT_VECTOR4 );

	mApp->registerPinJoiner( PID_VECTOR3, NID_JOIN_VECTOR3 );
	mApp->registerPinJoiner( PID_VECTOR4, NID_JOIN_VECTOR4 );

	return( INIT_OK );
}

void MathPlugin::deinitialise( void )
{
	mApp->unregisterPinClasses( PinClasses );

	mApp->unregisterNodeClasses( NodeClasses );

	mApp->unregisterInterface( IID_MATH );

	mApp = 0;
}

void MathPlugin::registerMetaTypeMathOperator(QMetaType::Type pType, MathInterface::MathOperator pOperator, MathInterface::MathOperatorFunction pFunction)
{
	mMathFunc.insert( MathFuncPair( pType, pOperator ), pFunction );
}

void MathPlugin::deregisterMetaTypeMathOperator(QMetaType::Type pType, MathInterface::MathOperator pOperator )
{
	mMathFunc.remove( MathFuncPair( pType, pOperator ) );
}

MathInterface::MathOperatorFunction MathPlugin::findMetaTypeMathOperator(QMetaType::Type pType, MathInterface::MathOperator pOperator) const
{
	return( mMathFunc.value( MathFuncPair( pType, pOperator ), nullptr ) );
}
