# Noum-MayaRenderer
Solution for real-time rendering of a maya scene using an external application.
There's two parts, the first is a plugin for Maya which saves relevant data to a shared memory while working. 
The second part is the external application which reads the same data and reconstructs the Maya scene.

It's somewhat limited and can only reconstruct meshes using vertices. 
It can also transmit transform operations, materials, and textures.

https://www.youtube.com/watch?v=tJmvl99yMUs&feature=youtu.be
