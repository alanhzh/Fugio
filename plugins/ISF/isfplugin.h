#ifndef ISFPLUGIN_H
#define ISFPLUGIN_H

#include <QObject>
#include <QDir>

#include <fugio/core/uuid.h>
#include <fugio/global_interface.h>
#include <fugio/plugin_interface.h>

using namespace fugio;

class ISFPlugin : public QObject, public fugio::PluginInterface
{
	Q_OBJECT
	Q_INTERFACES( fugio::PluginInterface )
	Q_PLUGIN_METADATA( IID "com.bigfug.fugio.isf.plugin" )

public:
	explicit ISFPlugin( void );

	virtual ~ISFPlugin( void ) {}

	virtual bool hasContext( void );

	static inline ISFPlugin *instance( void )
	{
		return( mInstance );
	}

	static bool hasContextStatic( void );

	inline static fugio::GlobalInterface *app( void )
	{
		return( instance()->mApp );
	}

	QString pluginPath( const QUuid &pUuid ) const
	{
		return( mPluginUuid.value( pUuid ) );
	}

	//-------------------------------------------------------------------------
	// fugio::PluginInterface

	virtual InitResult initialise( fugio::GlobalInterface *pApp, bool pLastChance );

	virtual void deinitialise( void );

private:
	void scanDirectory( ClassEntryList &pEntLst, QDir pDir, QStringList pPath = QStringList() );

private:
	static ClassEntry				 mNodeClasses[];
	static ClassEntry				 mPinClasses[];

	static ISFPlugin				*mInstance;

	fugio::GlobalInterface			*mApp;

	ClassEntryList					 mPluginClassEntry;

	QMap<QUuid,QString>				 mPluginUuid;
};

#endif // ISFPLUGIN_H