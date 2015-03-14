# sole
Simple Outline Editor

Many people who have used Maxthink for MSDOS like both the software and the manual.
Its developement was discontinued, and then Maxthink for Windows appeared, but it wasn't the same at all.
Some folks have managed to run Maxthink for MSDOS under DOSBox, or preferrably DOSEmu, but it's still proprietary.

Could there be a FOSS clone?
http://imaginethecanaries.blogspot.com.es/2015/03/thinking-and-cothinking-with-outline.html
A sole.c that used ncurses for MSDOS was created until Dec 1999, and kindly unfrozen in Mar 2015.

A year ago, Neil Larson gave someone permission to post the manual (a real gem, go download it and look at it) in PDF.
https://awarewriter.wordpress.com/2014/03/08/maxthink-max94-manual-is-here/

What next? I guess we should upload sole.c and DEMO.OL and see where we go from here.
I (LucasGSC) would like advice on language (free as in freedom, multiplatform, multilanguage, compilable).
Could we eventually run a crowdfunding process to help coders get to the first running instance in less than 15 years?
I think the sequence could be: viewer, basic editor, basic maxthink-like functionality (MLF), extended MLF, dream on.

## Requirements

 * ncurses
 
## Compile

~~~
$ gcc -lncurses -osole sole.c
~~~

## Usage
 
Use the next keys to move the cursor:

 * h move left
 * j move down
 * k move right
 * u move up
