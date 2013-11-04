Filer Rule-Making Reference

You will probably find most parts of the Filer pretty self-explanatory, but to really get power out of it, you'll want to read the information below. There are also a number of useful examples to get you going.


Rule Conditions

You will need at least one condition for the rule to test for. It can be the type of file, something about its name, how big it is, or some other attribute. These other attributes can be things like someone's Nickname kept in a Person file or the e-mail address in the To: field of an e-mail. Note that these can appear on just about any kind of file, but generally will only be found on the kind of file you expect it to be on. A rule will only match if all the conditions you set are met. 


Rule Actions

If the conditions are met, the Filer will perform a series of actions that you choose. Actions can be chained together, such as renaming a file and then moving it to another folder.

Move it to...	- Move a file to the folder entered in the box

Copy it to...	- Copy a file to the folder entered in the box

Rename it to...	- Rename the file. 

Move it to the Trash	- For those files which you no longer want.

Delete it		- Only if you're sure of yourself and hate a cluttered Trash can.

Terminal command...	- For experts. Run a command just as if you typed it into a Terminal. Substitutions (see below) are performed before the command is executed. This can make the Filer automatically do all sorts of things it couldn't do otherwise. If you move or rename the file this way, you'll need to do everything else with more Terminal command actions or a shell script.


Substitutions

You can also substitute certain information about the file into the text box for an action. For example: %TIME% is replaced with the current time before the action is performed. Here are all the possible choices:

%FILENAME%		- full name of the file being processed.

%EXTENSION%	- just the extension of the file, as in .txt in MyTextFile.txt or .tar.gz in MyArchive.tar.gz.

%BASENAME%	-  file name without extension, like MyTextFile in MyTextFile.txt.

%FOLDER%		- The full location of the folder which contains the file, like /boot/home/Documents for /boot/home/Videos/HaikuRocks.wmv

%FULLPATH%		- The full location of the file, such as /boot/home/config/MyFavoriteSong.mp3. You'll need this for Terminal Command actions. 
	
%DATE%			- The current date in the format MM-DD-YYYY

%EURODATE%		- The current date in the format DD-MM-YYYY

%REVERSEDATE%	- The current date in the format YYYY-MM-DD. This is often useful for file archives or for pictures.

%TIME%			- The current time using 24-hour time

%ATTR:xxxx%		- An extended attribute of the file. The technical name for the attribute is put between the colon and the second %. At this point, unfortunately, the case-sensitive, technical name of the attribute must be used. For example, an e-mail address attribute is META:email. This can be found in the FileTypes preferences application by choosing the type of file it is normally found on and double-clicking on it in the Extra Attributes box. In the window that appears, it will be in the box marked Internal Name. The most common ones are listed below.

Common Attributes and Names:

E-mail Address:	MAIL:email
E-mail Subject:		MAIL:subject
E-mail To:			MAIL:to
E-mail From:		MAIL:from
E-Mail Spam/Genuine:	MAIL:classification
Nickname:		META:nickname
Street Address:	META:address
MP3 Artist:		Audio:Artist
MP3 Album:		Audio:Album
MP3 Track Name:	Audio:Title
Program Category:	META:category


Example Rules


Move all e-mails on the MyAccount account to its own folder in the mail folder.

Email:Account is MyAccount
Move it to... /boot/home/mail/MyAccount


Sort JPEG photos by date into their own folder in /boot/home/Pictures

Type is jpeg
Rename it to... Photo %TIME%.jpg
Move it to... /boot/home/Pictures/%DATE%


Make sure that MP3's have searchable attributes using Axel Dörfler's excellent id3attr program, rename the to ArtistName - SongName.mp3, and sort them in the /boot/home/music folder by the artist's name. This would make importing an MP3 collection from somewhere else very easy. :)

Name ends with .mp3
Terminal command... id3attr '%FULLPATH%'
Rename it to... %ATTR:Audio:Artist% - %ATTR:Audio:Title%.mp3
Move it to... /boot/home/music/%ATTR:Audio:Artist%


Extract Zip archives to the Desktop and dump them into the Trash for later disposal

Name ends with .zip
Terminal command... unzip %FULLPATH% -d boot/home/Desktop
Move it to the Trash
