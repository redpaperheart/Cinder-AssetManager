AssetManager

AssetManager is a singleton class that helps loading and reusing external images and videos.

Our goal was to create a class that helps to easily preload images and keep them in memory so they are immediately available and easy to reuse later on. The class loads images either in the same or a separate thread and returns pointers to the textures. We used the AssetManager class heavily to preload folders of images that were used as texture sequence animations. Once loaded we could easily create and play multiple instances of the same texture sequence. 


The class is heavily inspired by (and includes code from) the thread loading discussion on the cinder forums. http://forum.libcinder.org/#topic/23286000000108063