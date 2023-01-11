# LasToVertex
A simple and lightweight library for reading las binary files directly into vectors for use in 3D engines. This was initially implemented by me for [Floof-Sim](https://github.com/STEN1/Floof-Sim), and is shared here as an easy to implement standalone version.

The whole reason for creating this, was to avoid having to either include huge libraries for being able to read las files directly, or needing to convert the lasfiles to .txt files (with really long load times) using lastools.
 
As of now, the libarary supports Point Data Record Format 1 (GPS Time) and Point Data Record Format 2 (Color Channels), with no immediate plans on including more versions, as this was made for a specific project, but feel free to make a request.

The implementation was made using the official [LAS specification](https://www.asprs.org/wp-content/uploads/2010/12/LAS_1_4_r13.pdf)
