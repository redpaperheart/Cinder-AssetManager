//
//  AssetManager.h
//
//  Created by Daniel Scheibel on 12/5/11.
//  Copyright (c) 2011 Red Paper Heart. All rights reserved.
//
#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Thread.h"
#include <string>


using namespace ci;
using namespace ci::app;
using namespace ci::qtime;
using namespace gl;
using namespace std;



class AssetManager {
  public:
    static AssetManager* getInstance();

    void setup();
    void update();
    string getAssetPath();
    Texture* getTexture( string path );
    MovieGl* getMovieGL( string path );
    vector<Texture *> getTextureListFromDir( string filePath );
    
    //static int numberOfConcurrentThreads;
    
    boost::shared_ptr<boost::thread>	mThread;
    
  private:
    AssetManager(){};                               // Private so that it can  not be called
    ~AssetManager();
    AssetManager(AssetManager const&){};            // copy constructor is private
    AssetManager& operator=(AssetManager const&){return *m_pInstance;}; // assignment operator is private
    static AssetManager* m_pInstance;
    
    map<string, Texture> textureAssets;
    string assetPath;
    
	boost::mutex loadedSurfacesMutex;
    map<string, Surface> loadedSurfaces;
    
	deque<boost::shared_ptr<boost::thread> > threads;
    
    boost::mutex urlsMutex;
    deque<string> urls;
    
    bool isSetup = false;
    
    void threadLoad();
	//void threadLoad(const std::string url);
    
};