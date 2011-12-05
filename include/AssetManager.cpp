#include "AssetManager.h"

using namespace boost;
using namespace ci;
using namespace ci::app;
using namespace ci::qtime;
using namespace gl;
using namespace std;

AssetManager* AssetManager::m_pInstance = NULL;
bool AssetManager::useResources = false;
int AssetManager::numberOfConcurrentThreads = 2;

AssetManager* AssetManager::getInstance(){
    if (!m_pInstance)
        m_pInstance = new AssetManager;
    return m_pInstance;
}

string AssetManager::getAssetPath(){
    if(assetPath.empty()){
        assetPath = getAppPath();
        assetPath = assetPath.substr( 0, assetPath.find_last_of("/")+1 );
        //console() << "set assetPath: " << assetPath << endl;
    }
    return assetPath;
}

Texture* AssetManager::getTexture( string path){
    if(textureAssets[path]){
        return &textureAssets[path];
    }else{
        textureAssets[path] = Texture(0,0);
        if(threads.size() > numberOfConcurrentThreads){
            urls.push_back(path);
        }else{
            threads.push_back(boost::shared_ptr<thread>( new thread(&AssetManager::threadLoad, this, path) ) );
        }
        return &textureAssets[path];
    }
}

MovieGl* AssetManager::getMovieGL( string url){
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
    vector<Texture *> textures;
    textures.clear();
    fs::path p( filePath );
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
    return textures;
}


void AssetManager::update(){
	// currently update needs to be called from outside.
    // in the future this should run on it's own internal timer.
    
    if(!threads.empty()){
        // clean all finished threads:
        deque<boost::shared_ptr<boost::thread> >::iterator itr;
        for(itr = threads.begin(); itr != threads.end();){
            if( (*itr)->timed_join<posix_time::milliseconds>(posix_time::milliseconds(1)) )
                itr = threads.erase(itr);
            else
                ++itr;
        }
    }
    // start loading all outstanding images:
    while (!urls.empty() && threads.size() < numberOfConcurrentThreads ){
        threads.push_back(boost::shared_ptr<thread>( new thread(&AssetManager::threadLoad, this, urls.back()) ) );
        urls.pop_back();
    }
    if(!loadedSurfaces.empty()){
        // create texture from finished surfaces
        loadedSurfacesMutex.lock();
        for ( map<string, Surface>::iterator mitr=loadedSurfaces.begin(); mitr != loadedSurfaces.end(); mitr++ ){
            textureAssets[(*mitr).first] = Texture( (*mitr).second );
        }
        loadedSurfaces.clear();
        loadedSurfacesMutex.unlock();
    }
}

void AssetManager::threadLoad(const string url){
	Surface surface;
	ImageSourceRef image;
    
	//console() << getElapsedSeconds() << ":" << "Loading:" << url << endl;
    
    try{
        //try loading from resource folder
        image = loadImage( loadResource( url ) );
    }catch(...){
        try { 
            // try to load relative to app
            image = loadImage( loadFile( getAssetPath()+url ) ); 
        }
        catch(...) { 
            try {
                // try to load from URL
                image = ci::loadImage( ci::loadUrl( Url(url) ) ); 
            }
            catch(...) {
                console() << getElapsedSeconds() << ":" << "Failed to load:" << url << endl;
                return;
            }
        }
    }
    
	// allow interruption now (robust version: catch the exception and deal with it)
	try { 
		boost::this_thread::interruption_point();
	}
	catch(boost::thread_interrupted) {
		// exit the thread
		return;
	}
	// create surface
	surface = Surface(image);
    
	boost::this_thread::interruption_point();
    
	// resize
//	Area source = surface.getBounds();
//	Area dest(0, 0, mMaxSize, mMaxSize);
//	Area fit = Area::proportionalFit(source, dest, false, false);
//    
//	if(source.getSize() != fit.getSize())
//	{
//		console() << getElapsedSeconds() << ":" << "Resizing surface..." << endl;
//		surface = ip::resizeCopy(surface, source, fit.getSize());
//		console() << getElapsedSeconds() << ":" << "Surface resized" << endl;
//	}
    
	// allow interruption now
	boost::this_thread::interruption_point();
    
	// copy to main thread
	loadedSurfacesMutex.lock();
	loadedSurfaces[url] = surface;
	loadedSurfacesMutex.unlock();
    
	//console() << getElapsedSeconds() << ":" << "Thread finished" << endl;
}

