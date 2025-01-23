## ManeleMax

### About

*ManeleMax* is a Windows application that, while running, it forces the system volume at 100% if you are playing manele on your computer. If you are playing any type of music other than manele, the system volume is forced at a low level (25%).  It works by leveraging the [**Windows.Media.Control**](https://learn.microsoft.com/en-us/uwp/api/windows.media.control?view=winrt-26100) API from WinRT, so it will detect songs that play from programs which are using said API (i.e. any modern web browser). This is also the reason why it will not work on any Windows older than 10 (version 1809).

### How to build

From a developer command prompt, run the following commands:
```sh
mkdir build
cmake --preset x64-release -S . -B build
cmake --build build
```

You can also download the already compiled binary from the releases page.

_Note: To be able to build, you need to install **C++ WinUI app development tools**. Modify you Visual Studio install to include this component as well. This is required by the WinRT APIs._

### How to run
Just run the executable. If you see that a new system tray icon has appeared which looks like Florin Salam's face, then it's working. To close it, right click on the system tray icon and select the _Exit_ option from the context menu.

To run it everything you log in to your computer, just create a shortcut in
`%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup`

### FAQ

***Q1:** What is manele?*

**A1:** See the [wikipedia page](https://en.wikipedia.org/wiki/Manele).

***Q2**: Why did you create this?*

**A2:** For fun. I guess you could use this to troll your friends that hate manele by installing *ManeleMax* on their computers. They deserve it.

***Q3**: Is there a way to customize the volume levels?*

**A3:** No, you should listen to manele at 100% volume! You do want your neighbors to also hear it, right? The other genres should be listened at low volume levels in order to not damage your hearing.

***Q4**: Some songs are not detected. How can I customize the list of detected keywords to add the missing artists?*

**A4:** Unfortunately, this is not possible right now because that list is hardcoded in the binary. You can submit a pull request to update the list and I will review it.

***Q5:** This program forces my volume down even though I'm not listening to music, but I'm watching a documentary about fish on YouTube instead. How can I fix this?*

**A5:** Unfortunately, *ManeleMax* is limited by what information Windows is given by the web browser. There is no way to verify if you are watching on YouTube is a music video. The only solution is to just close *ManeleMax* temporarily. 

***Q6**: I'm listening manele on VLC media player. Why doesn't this program work?*

**A6:** Unfortunately, VLC does not use the **Windows.Media.Control** API to inform Windows about the currently playing media and the playback status.

***Q7**: Will you port this for other operating systems?*

**A7:** If it's something in demand, maybe when I will have some more free time. I'm almost sure that on Linux you can achieve this with just a shell script.
