# SteaScreeLoaded Changelog

## 1.23.0
Added a setting to disable the footer.

## 1.22.0
- No automatic update check in offline mode anymore.
- The preview area now displays the creation time of each screenshot.

## 1.21.0
- Should a settings file from an earlier version be used, in which the API key was still stored unencrypted, it is now immediately usable and automatically encrypted. Previously, the key had to be manually re-entered. 
- The game-sources mode formerly named "None (Local only)" is now called "Offline mode" and also uses the `games.json` in the programme directory, if present.
- "Local JSON file" is now called "Download JSON file" and behaves like Offline mode, but unlike it, establishes Internet connections to check the availability of the JSON file and download it if it is not already present.

## 1.20.1
Footer and About section revised.

## 1.20.0
API key obfuscation added: the key is now encrypted and decrypted using [Tiny-AES](https://github.com/kokke/tiny-AES-c) (AES-256), so it no longer appears in plain text in the settings file. Since the encryption key is not tied to a specific user (no individual password is required), this does not protect against determined attackers, but it prevents the merely curious, basic malware, scripts, third-party applications and file scanners from reading it easily and generating unwanted traffic that could trigger the key's suspension. As the Steam API key does not permit any security-critical actions, this should be an appropriate safeguard that imposes no additional burden on the user.

## 1.19.1
Corrected the order of the preview images: the oldest images are on the left and the newest on the right. "Age" does not refer to the date of copying or uploading, but to the moment the original file was created. This moment determines the file name in the screenshot folder, and that file name in turn forms the basis of the sorting. This means that if an older image is copied into the screenshot folder using the programme, it will not appear on the far right, but in the position corresponding to its timestamp. When switching the game in the game-ID list and when starting the programme, the view is first moved to the newest image on the far right, which quite naturally is not necessarily the one most recently copied.

## 1.19.0
Load game names more reliably and quickly: Under "Games source", you can now select "Local JSON file". This will download a JSON of roughly 12 MB into the programme folder. Once present, the game names will be immediately available at every start.
The JSON is a stripped-down mirror of the old API [ISteamApps/GetAppList/v1](https://api.steampowered.com/ISteamApps/GetAppList/v1), meaning it may be out of date, but in most cases it should currently be the best option. The JSON schema is `{"ID": "Name", "ID2": "Name 2"}`, for example:

`{
    "10": "Counter-Strike",
    "100": "Counter-Strike: Condition Zero Deleted Scenes",
    "10000": "Enemy Territory: Quake Wars",
    "1000000": "ASCENXION",
    "999990": "Bouncing Hero"
}
`

## 1.18.0
- Workflow enhancement: A check has been implemented to determine whether Steam is still running. If the Steam process is detected, or if there is no method available to check for the operating system in use, a warning message will appear. If the Steam process is not found, the "Prepare screenshots for uploading" button now functions immediately. This removes the need for the manual confirmation that was previously required each time.
- The scrollbar now moves directly to the most recent image (maximum position) after copying images.
- Loading of the footer has been moved from the constructor to the bootstrap function; otherwise, the footer would be ignored when the user interface is reset.
- The footer has been revised. Licence information is now shown only in the About window.
- Shortcuts have been added for the menu bar.

## 1.17.0
- Reversed the order of the screenshot preview so that the most recent image in the folder appears first.
- Copying images into the screenshot folder via the Copy button now refreshes the preview, causing the newest image to be displayed immediately.
- Reworked the About window: links and the QR code are now clickable and the version number is displayed.

## 1.16.0
- Accordion effect remedied. Previously, elements of the user interface could be pressed into one another by automatic resizing and would overlap, causing them to become unintentionally concealed.
- Status bar added.
- Menu added: Update, Settings (Open/Reset/Delete), About, Quit.

## 1.15.1
Bug fixed: After restarting, the API settings remained visible even when the selected API did not require any settings.

## 1.15.0
- API settings are now only displayed when required.
- Next to the API selection, the server availability is now shown: green dot = reachable, red circle = not reachable.

## 1.14.1
API switching bug fixed: A redeclaration of a global variable (the index of the API selection list) as a local variable within a method had prevented reliable switching between APIs. The index loaded from the settings file appeared to be applied, yet was in fact not used by the programme. Instead, the default index was taken. This created the impression that the open Steam API V2 was functioning once more, although it is presently unreachable.

## 1.14.0
- To spare the clicking finger, the buttons for jumping to the next/previous preview image were replaced with a scroll bar.
- Potential division-by-zero errors in thumbnail generation have been handled.
- Thumbnail sizes have been set to a maximum edge length of 200 pixels. Previously, only the width was limited, allowing portrait thumbnails to exceed the 200-pixel standard. Even though Steam currently accepts oversized thumbnails, we stick to the standard to ensure they cannot exceed the 200x200 frame.

## 1.13.0
- Added a "None" option to skip all external API calls in "Game source" for those who wish to work only locally with IDs.
- Added a button to reset a custom naming, which deletes the chosen name mapping from ids.txt.
- Fixed a scaling issue of the preview screenshot after startup.

## 1.12.1
Bug fix: Endless "loading..." in the game ID list and no IDs on first launch.

## 1.12.0
- Selection of the source (API) from which the game names shall be downloaded.
- Button for removing the Steam API key from the settings.

## 1.11.0
Quick fix for the games list: It is now possible to add your own [Steam API key](https://steamcommunity.com/dev/apikey).
Background: Steam has closed open access to [API v2](https://api.steampowered.com/IStoreService/GetAppList/v2/) without prior notice. As a result, game names can no longer be fetched from the original source. This may happen from time to time. With a personal API key, however, it is possible to load many of the missing names from the older [API v1](https://api.steampowered.com/IStoreService/GetAppList/v1/).

![2025-11-14 16_40_50-SteaScreeLoaded](https://github.com/user-attachments/assets/29d25146-e0d5-42dd-8f84-dd7e59892bc7)

## 1.10.1
Steam API requests have been switched from http to https. This is the officially recommended option and offers a degree of protection against interception, falsification, and manipulation of connection data through encryption of data transmission.

## 1.10.0
Autocomplete added for the Game ID input field. As soon as a game or ID is typed, matching entries from the list are immediately suggested for selection.

## 1.9.0
- Arrow buttons for navigating the preview image added
- The preview header displays the title of the selected game
- Below the preview image, the total number of screenshots in the selected folder and the index of the currently shown image are displayed
![2025-11-12 10_17_13-SteaScreeLoaded](https://github.com/user-attachments/assets/a11a6686-c55d-4d25-bec5-7daea601e3dd)

## 1.8.0
**Renaming feature added: No more unnamed games.**
Non-Steam games added as shortcuts in the Steam client often appear in the list without a name. The same applies to games that no longer exist on Steam under their original ID. To address this, a new feature now allows you to assign names to previously unnamed game IDs. Custom names are stored locally in the file `ids.txt`. When the programme starts, all IDs are checked against this list and are automatically given their assigned names.

![2025-11-11 09_20_22-Greenshot](https://github.com/user-attachments/assets/0cec3edd-7951-424e-8842-631b22396a05)

## 1.7.1
- Preview image resizing corrected. The size of the image button now adjusts dynamically to the window dimensions.
- A splitter has been added, dividing the user interface vertically into two halves to allow the image size to be modified.

## 1.7.0
The preview image of the selected game now opens the corresponding screenshot folder when clicked.

## 1.6.2
- New update feature that retrieves the latest version from GitHub
- Footer information

## 1.6.1
User interface more clearly organised

## 1.6.0
- Image preview: Some games appear in the list only with their ID number. To make folder selection easier, an image is now displayed as soon as a game/ID is selected in the list. The user interface has been rearranged accordingly.
- Path to the saved settings adjusted to the new programme (fork) name.
- Default JPEG quality increased from 95 to 100. A value somewhere in the 90s might offer a better balance between quality and file size, and the difference is barely visible to the naked eye, yet JPEG remains lossy even at 100 %. Since the image source is often already compressed, there is little reason to further reduce the quality by default.


## 1.5.5
First fork version (SteaScree -> SteaScreeLoaded) aimed at fixing several bugs.
- The current Steam profile names are now read from localconfig.vdf using a new method; the previous method sometimes produced stale/outdated names and is retained only as a safety fallback if the new method fails.
- Unused status bar removed.
- Made compilable for Qt 6.5.3.

## 1.5.4
* Qt 5.9.1.
* Added "JPEG Quality" spinbox for situations, when SteaScree converts image to JPEG. It happens if the input image is too large for Steam Cloud or if it has non-JPEG format. Defaults to 95.
* Slight redesign. Status field is now visible all the time.

## 1.5.3
* Now the app switches to debugging to file automatically, when the it is not ran in Qt Creator.

## 1.5.2
* Added a logic to save debug info to the debug.log file.
* Fixed the bug when the app was crashing, if the Steam user personal name (nickname) was not found in config files. Closes issue #16.
* Removed some unused methods.

## 1.4.1
* Minor UI improvement.

## 1.4.0
* Added Drag&Drop capability.
* Fixed a bug when the app crashed if QImage object was not created if the image file is corrupted or have a wrong extension. Added a new status message if there were such files in the queue and some such files were not copied.
* Fixed misleading status message when userdata directory was not found.
* Some additional minor fixes.

## 1.3.3
* Fixed a bug when personal names were not populated to the respective combobox, if there are multiple config.cfg files were found for a single user. This is completely normal, and now personal name ges extracted from the first found config. Typically, even if there are multiple configs, personal names in them are exactly the same.
* Removed some unused methods.

## 1.3.2
* Implemented check for the encoding process of incoming JPEG files. If it is a progressive, not baseline, pristine copy of file is saved to the filesystem. Otherwise, the file is internally processed by Qt, just like PNG and other supported formats. Closes issue #9.

## 1.3.1
* Well, seems like user switching never worked right. Now it does. Games listed in the combobox change on-the-fly when user ID get changed.

## 1.3.0
* Personal name is now shown in the combobox alongside user ID. Useful when a computer is shared among a number of Steam users.

## 1.2.0
* Added check for an updated version. There is an option to never offer updates.
* Tooltips.
* Minor UI improvement.

## 1.1.0
* Implemented checking of dimensions of provided screenshots. If exceptionally large screenshot is detected, now SteaScree offers user a choice to auto-resize it to the maximum size allowed by Steam (maintaining aspect ratio), skip it or even try to upload it nevertheless. Closes issue #4.
* Some bugs fixed.
* Minor UI improvement.

## 1.0.6
* Fixed a bug when screenshots weren't added to the VDF-file if latest copied screenshot was a dupe.
* Minor UI improvement.

## 1.0.5
* Significantly improved quality of generated thumbnails. Now they are even better and smoother, than those generated by Steam itself. Closes issue #1.

## 1.0.3
* Fixed a bug where screenshots with identical creation timestamps were not copied. Now, if the timestamp overlaps for several screenshots, each of them has identical basename, but different incremental integer after the underscore and before ".jpg" extension.
* Button padding are now set in-code for a more convenient binary building across different platforms.

## 1.0.2
* Initially I thought Steam generates missing thumbnails automatically. This was not the case, so since this release SteaScree takes care of them.
* Minor UI improvement.

## 1.0.1
* Game ID combo box is now editable.
* Minor UI improvement.

## 1.0.0

* Initial public release.
