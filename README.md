# Thank you section!

Special thanks to pixelsnafu's awesome volumetric cloud resource list here: https://gist.github.com/pixelsnafu/e3904c49cbd8ff52cb53d95ceda3980e

and special thanks to the volumetric cloud projects from 

- NadirRoGue: https://github.com/NadirRoGue/RenderEngine
- clayjohn: https://github.com/clayjohn/realtime_clouds
- fede-vaccaro: https://github.com/fede-vaccaro/TerrainEngine-OpenGL

If you are interested in the topic of realtime volumetric cloud rendering, checkout all the resources from above in addition to this project, in order to get a complete picture.

# OpenGL Realtime Volumetric Cloud Renderer

This is a demo application which renders volumetric clouds in realtime through raymarching, combined with the preetham atmospheric scattering model, a free flight camera in order to fly through the clouds and an ImGUI UI Interface to control the rendering process. It is a simple, well commented and barebones project with all dependencies included which aims to demonstrate this specific volumetric cloud rendering technique to those interested. Just download the project and run the .exe file in the /bin folder to start the demo.

This volumetric clouds implementation is based on Andrew Schneiders article "Real-Time Volumetric Cloudscapes" from the book GPU Pro 7: Advanced Rendering Techniques (2016) and his follow-up presentations (see the above resource list for links). 
However, two components from the article are not used in this implementation:

1. Frame-reprojection. Reason being, that it is not that easy to implement (sorry for that :(, but the other projects above have implementations for it. Instead of frame-reprojection, a simple upscale-shader is used (thanks to clayjohns project for that), which takes a pixel color from the quarter-resolution cloud colorbuffer and assigns that same color to a 4x4 pixel block in the fullscreen main colorbuffer. This results in a little blurring, but it works very well for this demo (see the screenshots below).
2. A weathermap texture. Instead, a cloudType float value determines the current cloud type (ranging from stratus to cumulus) and a cloudCoverage float value determines the cloud coverage of the whole sky. This makes it easier to understand and easier to play around with those values through the UI settings.  


# Screenshots

### Cloudscapes

![Cloudscape01](./Screenshots/Cloudscape01.PNG?raw=true)
![Cloudscape02](./Screenshots/Cloudscape02.PNG?raw=true)
![Cloudscape03](./Screenshots/Cloudscape03.PNG?raw=true)
![Cloudscape04](./Screenshots/Cloudscape04.PNG?raw=true)
![Cloudscape05](./Screenshots/Cloudscape05.PNG?raw=true)
![Cloudscape06](./Screenshots/Cloudscape06.PNG?raw=true)
![Cloudscape07](./Screenshots/Cloudscape07.PNG?raw=true)
![Cloudscape08](./Screenshots/Cloudscape08.PNG?raw=true)

### ImGUI UI Settings Window

![Raymarch Settings](./Screenshots/ImGUI_Settings.PNG?raw=true)








