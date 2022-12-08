\mainpage <br>
Collapsed Fragment Game Engine Documentation <br>
Courtesy of GSAS 4550 taught by Chris McEvoy

My final project was to implement Doxygen into the game engine to <br>
accomodate generation of external documentation. Specifically, I  <br>
wanted to accomplish: <br>
=> Creating a sensible config file that could be immediately adopted. <br>
=> Finding and converting existing documentation into a suitable, uniform style <br>
=> Investigate automated generation of documentation <br>
=> Investigate automated uploading of documentation to a database <br>

I was fairly successful in these tasks, most notably in generating the documentation on build!
I was unable to devise a system for uploading the documentation to a non-github central location.

Due to some significant technical difficulties, I was unable to do this project in Visual Studio, so some of my project settings may not work externally, especially the automated Doxygen generation on Build.
I did this using the Post-Build Event function, present in Visual Studio as well as the IDE I ended up using (Rider), so it would not be difficult to recreate if I needed to.
[Here is the source I used to figure that out](https://learn.microsoft.com/en-us/visualstudio/ide/how-to-specify-build-events-csharp?view=vs-2022).
The specific command I used was <br>`call $(SolutionDir)../docs/Doxygeneration/doxygen-1.9.5/build/bin/doxygen  $(SolutionDir)../docs/Doxygeneration/doxygen-1.9.5/build/bin/config_file.txt`<br>
and the hierarchy should be consistent regardless of the IDE, only the project settings might not transfer properly.

The Doxygen output file is located at `$(SolutionDir)../docs/Doxygeneration`. Due to some complications with compiling Doxygen, the actual Doxygen files are further within that directory (/doxygen-1.9.5/build/bin). I ask that you ignore that.

Comments primarily use
=> brief, return, see/sa, remark(s), in that order.
=> params are only documented specifically if they have some trait that would also reasonably be considered a remark.



