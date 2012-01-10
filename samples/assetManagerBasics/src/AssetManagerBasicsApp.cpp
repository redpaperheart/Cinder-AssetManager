/*
 *  AssetManagerBasicsApp.cpp
 *
 *  Created by Daniel Scheibel on 12/5/11.
 *  Copyright (c) 2011 Red Paper Heart. All rights reserved.
 *  http://www.redpaperheart.com
 * 
 */

#include "AssetManager.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AssetManagerBasicsApp : public AppBasic {
  public:
	void setup();
	void update();
	void draw();
    
    Texture *textureFromRelativePath;
    Texture *textureFromResources;
    
};

void AssetManagerBasicsApp::setup(){
    //load in thread
    textureFromRelativePath = AssetManager::getInstance()->getTexture( "data/relativeImage.png" );
    textureFromResources = AssetManager::getInstance()->getTexture( "data/resourceImage.png" );
}

void AssetManagerBasicsApp::update(){
    AssetManager::getInstance()->update();
}

void AssetManagerBasicsApp::draw(){
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    gl::enableAlphaBlending();
    gl::color(1,1,1);
    
    gl::draw( *textureFromResources );
    gl::draw( *textureFromRelativePath );
    
}


CINDER_APP_BASIC( AssetManagerBasicsApp, RendererGl )
