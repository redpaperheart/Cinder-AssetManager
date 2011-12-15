#include "AssetManager.h"

using namespace boost;
using namespace ci;
using namespace ci::app;
using namespace ci::qtime;
using namespace gl;
using namespace std;

AssetManager* AssetManager::m_pInstance = NULL;
//int AssetManager::numberOfConcurrentThreads = 2;

AssetManager* AssetManager::getInstance(){
    if (!m_pInstance)
        m_pInstance = new AssetManager;
    return m_pInstance;
}

AssetManager::~AssetManager(void){
    // stop loader thread and wait for it to finish
    mThread->interrupt();
    mThread->join();

    // clear buffers
    textureAssets.clear();
    urls.clear();
    loadedSurfaces.clear();
}

void AssetManager::setup(){
    if(!isSetup){
        mThread = boost::shared_ptr<boost::thread>(new boost::thread(&AssetManager::threadLoad, this));
        isSetup = true;
    }
}

string AssetManager::getAssetPath(){
    if(assetPath.empty()){
        assetPath = getAppPath();
        assetPath = assetPath.substr( 0, assetPath.find_last_of("/")+1 );
        //console() << "set assetPath: " << assetPath << endl;
    }
    return assetPath;
}

Texture* AssetManager::getTexture( string url, bool loadInThread ){
    setup();
    if(textureAssets[url]){
        return &textureAssets[url];
    }else{
        textureAssets[url] = Texture(1,1);
        if(loadInThread){
            textureAssets[url] = Texture(1,1);
            urlsMutex.lock();
            urls.push_back(url);
            urlsMutex.unlock();
        }else{
            try{
                //try loading from resource folder
                textureAssets[url] = Texture( loadImage( loadResource( url ) ) );
            }catch(...){
                try { 
                    // try to load relative to app
                    textureAssets[url] = Texture( loadImage( loadFile( getAssetPath()+url ) ) ); 
                }
                catch(...) { 
                    try {
                        // try to load from URL
                        textureAssets[url] = Texture( loadImage( loadUrl( Url(url) ) ) ); 
                    }
                    catch(...) {
                        console() << getElapsedSeconds() << ":" << "Failed to load:" << url << endl;
                    }
                }
            }
        }
        return &textureAssets[url];
    }
}

MovieGl* AssetManager::getMovieGL( string url){
    setup();
    MovieGl *m;
    try{
        //try load from resource folder
        m = new MovieGl( loadResource( url ) );
    }catch(...){
        try{
            //try load relative to app
            m = new MovieGl( getAssetPath() + url );
        }catch(...){
            console() << getElapsedSeconds() << ":" << "Failed to load MovieGL:" << url << endl;
        }
    }
    return m;
}

vector<Texture *> AssetManager::getTextureListFromDir( string filePath ){
    setup();
    vector<Texture *> textures;
    textures.clear();
    fs::path p( filePath );
    try{
    for ( fs::directory_iterator it( p ); it != fs::directory_iterator(); ++it ){
        if ( fs::is_regular_file( *it ) ){
            // -- Perhaps there is a better way to ignore hidden files
            string fileName =  it->path().filename().string();
            if( !( fileName.compare( ".DS_Store" ) == 0 ) ){
                textures.push_back( getTexture( filePath + fileName ) );
                //textureAssets[filePath + fileName] = Texture(1,1);
                //textures.push_back( &textureAssets[filePath + fileName] );
            }
        }
    }
    }catch(...){
        console() << "ERROR opening AssetManager::getTextureListFromDir( "<< filePath <<")"<<endl;
    }
    return textures;
}


void AssetManager::update(){
    
    loadedSurfacesMutex.lock();
    int count = 0;
    while (!loadedSurfaces.empty() && count < 3){
        // create texture from finished surfaces
//        for ( map<string, Surface>::iterator mitr=loadedSurfaces.begin(); mitr != loadedSurfaces.end(); mitr++ ){
//            textureAssets[(*mitr).first] = Texture( (*mitr).second );
//            loadedSurfaces.erase( (*mitr).first );
//            console()<< "created: " << (*mitr).first << endl;
//        }
        map<string, Surface>::iterator mitr=loadedSurfaces.begin();
        textureAssets[(*mitr).first] = Texture( (*mitr).second );
        //console()<< "created: " << (*mitr).first << endl;
        loadedSurfaces.erase( (*mitr).first );
        //loadedSurfaces.clear();
        count++;
    }
    loadedSurfacesMutex.unlock();
    
}
void AssetManager::threadLoad(){
	bool			empty;
	bool			succeeded;
	Surface			surface;
	ImageSourceRef	image;
	string			url;
    
	// run until interrupted
	while(true) {
		// fetch first url from the queue, if any	
		urlsMutex.lock();
		empty = urls.empty();
		if(!empty) url = urls.front();
		urlsMutex.unlock();
        
		if(!empty) {
			// try to load image
			succeeded = false;
            
			// try to load from FILE
			if(!succeeded) try { 
				image = ci::loadImage( ci::loadFile( getAssetPath()+url ) ); 
				succeeded = true;
			} catch(...) {}
            
			// try to load from URL
			if(!succeeded) try { 
				image = ci::loadImage( ci::loadUrl( Url(url) ) ); 
				succeeded = true;
			} catch(...) {}
            
			if(!succeeded) {
				// both attempts to load the url failed
                
				// remove url from queue
				urlsMutex.lock();
				urls.pop_front();
				urlsMutex.unlock();
                
				continue;
			}
            
			// succeeded, check if thread was interrupted
			try { boost::this_thread::interruption_point(); }
			catch(boost::thread_interrupted) { break; }
            
			// create Surface from the image
			surface = Surface(image);
            
			// check if thread was interrupted
			try { boost::this_thread::interruption_point(); }
			catch(boost::thread_interrupted) { break; }
            
			// resize image if larger than 4096 px
//			Area source = surface.getBounds();
//			Area dest(0, 0, 4096, 4096);
//			Area fit = Area::proportionalFit(source, dest, false, false);
//            
//			if(source.getSize() != fit.getSize()) 
//				surface = ci::ip::resizeCopy(surface, source, fit.getSize());
            
			// check if thread was interrupted
			try { boost::this_thread::interruption_point(); }
			catch(boost::thread_interrupted) { break; }
            
			// copy to main thread
            loadedSurfacesMutex.lock();
            loadedSurfaces[url] = surface;
            loadedSurfacesMutex.unlock();

			// remove url from queue
			urlsMutex.lock();
            urls.pop_front();
            urlsMutex.unlock();
		}
		// sleep a while
		boost::this_thread::sleep( boost::posix_time::milliseconds(10) );
	}
}

//void AssetManager::threadLoad(const string url){
//	Surface surface;
//	ImageSourceRef image;
//    
//	//console() << getElapsedSeconds() << ":" << "Loading:" << url << endl;
//    try{
//        //try loading from resource folder
//        image = loadImage( loadResource( url ) );
//    }catch(...){
//        try { 
//            // try to load relative to app
//            image = loadImage( loadFile( getAssetPath()+url ) ); 
//        }
//        catch(...) { 
//            try {
//                // try to load from URL
//                image = loadImage( loadUrl( Url(url) ) ); 
//            }
//            catch(...) {
//                console() << getElapsedSeconds() << ":" << "Failed to load:" << url << endl;
//                return;
//            }
//        }
//    }
//    
//	// allow interruption now (robust version: catch the exception and deal with it)
//	try { 
//		boost::this_thread::interruption_point();
//	}catch( boost::thread_interrupted ) {
//		// exit the thread
//		return;
//	}
//	// create surface
//	surface = Surface(image);
//    
//	boost::this_thread::interruption_point();
//    
//	// allow interruption now
//	boost::this_thread::interruption_point();
//    
//	// copy to main thread
//	loadedSurfacesMutex.lock();
//	loadedSurfaces[url] = surface;
//	loadedSurfacesMutex.unlock();
//    
//	//console() << getElapsedSeconds() << ":" << "Thread finished" << endl;
//}

