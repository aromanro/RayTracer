# RayTracer
A ray tracing program

Mainly based on 'Ray Tracing' books from Peter Shirley: http://in1weekend.blogspot.com/2016/01/ray-tracing-in-one-weekend.html
I skipped the boring (for me) texture generation part and also the motion bluring, I preffered to go further enhancing the materials and loading obj files.

But not only, info coming soon on the Computational Physics Blog: https://compphys.go.ro

Some papers that were used:

'An Anisotropic Phong BRDF Model' by Michael Ashikhmin and Peter Shirley
https://www.cs.utah.edu/~shirley/papers/jgtbrdf.pdf

Osada et All, 'Shape Distributions'
http://www.cs.princeton.edu/~funk/tog02.pdf

'Fast, minimum storage ray-triangle intersection'
Tomas Möller and Ben Trumbore. 
Journal of Graphics Tools, 2(1):21--28, 1997. 
http://www.graphics.cornell.edu/pubs/1997/MT97.pdf

The project has some known issues, for example loading an object from an obj file does not split correctly concave polygons, they must be convex.
Also importance sampling with composite objects has some issues. I know why they occur and how it could be solved, but I don't have enough patience for those changes right now :)


Some images obtained while implementing it:

After the first book, 'Ray Tracing in one weekend':

![Screenshot](img/RayTracing4000spp.png)

With local illumination, depth of field and textures:

![Screenshot](img/LocalIllumination.png)

The Cornell box (during the 'next week' book):

![Screenshot](img/CornellBox.png)

Adding triangles, already beyond the 'rest of your life' book:

![Screenshot](img/triangle.png)

Adding Beer-Lambert law for transparent objects:

![Screenshot](img/BeerLambert.png)

The first obj file loaded in the Cornell box:

![Screenshot](img/monkey.png)

Making it transparent:

![Screenshot](img/transparentmonkey.png)

Trying some other obj models:

![Screenshot](img/car.png)

Now together with colors/textures:

![Screenshot](img/indian.png)

Implementing specular reflection:

![Screenshot](img/Phong.png)

Trying it on a model, exponents are too high:

![Screenshot](img/IndianPhong.png)

More testing:

![Screenshot](img/carphong.png)

And some new model for testing the features:

![Screenshot](img/newcarphong.png)
