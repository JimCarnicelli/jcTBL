# jcTBL
Implementation of Eric Brill's "transformation-based learning" algorithm

## Nuts 'n' bolts
This C++ code is intended to be used as a static library. The demo program is 
largely my own hacking to test out what I've built.

I built this using Xcode 7.3.1 on a late 2013 iMac. I'm using the 
GNU++11 [-std=gnu++11] language dialect and the libc++
(LLVM C++ standard library with C++11 support) library. This don't 
have reliances on Boost or any other libraries outside the 
[STL](https://en.wikipedia.org/wiki/Standard_Template_Library). 
I haven't tried compiling this on Windows, Linux, or any other 
systems, but I see no reason why it wouldn't, given an appropriate 
makefile.

The library code can be found in the /jcTBL subdirectory in /include and 
/source. You'll want to include the libjcTBL.a static library and include 
/jcTBL/include/jctbl.hpp, which brings in all the other include files.

The jctbl.hpp include file's header is loaded with background and advice.

## About this project

I'm resuming my
[AI](https://en.wikipedia.org/wiki/Artificial_intelligence) /
[NLP](https://en.wikipedia.org/wiki/Natural_language_processing) 
research. In studying different kinds of 
[classifier systems](https://en.wikipedia.org/wiki/Learning_classifier_system)
I came across 
[Eric Brill's](https://en.wikipedia.org/wiki/Eric_Brill)
concept of 
"[transformation-based learning](http://dl.acm.org/citation.cfm?id=1073336.1073342&coll=GUIDE&dl=ACM)"
(TLB). There are various TLB implementations out there, including
[fnTLB](https://www.cs.jhu.edu/~rflorian/fntbl/). 
Sadly, I was unable to compile their code and struggled to understand 
its structure. Moreover, I learn better by doing, so I decided to build 
my own from scratch using various documents describing the algorithm 
and enhancements by other researchers along the way.

After creating a first prototype, I decided to start from scratch with a
cleaner implementation and an eye toward applying this to a variety of 
practical tasks, including 
[tokenization](https://en.wikipedia.org/wiki/Lexical_analysis#Tokenization),
[part of speech (PoS) tagging](https://en.wikipedia.org/wiki/Part-of-speech_tagging),
[chunking](https://en.wikipedia.org/wiki/Shallow_parsing),
[named-entity recognition](https://en.wikipedia.org/wiki/Named-entity_recognition),
and more. As of my first commit date (9/9/2016), I have only applied this
to a first task of PoS tagging using training and test sets drawn from the
[Penn Treebank](http://www.cis.upenn.edu/~treebank/).

At heart, this is a direct implementation of Brill's original algorithm as 
he described it. As he and others have noted, the training process can be 
quite slow. I took inspiration from work by Mark Hepple
([Independence and commitment: assumptions for rapid training and execution 
of rule-based POS taggers](https://aclweb.org/anthology/P/P00/P00-1036.pdf))
and Radu Florian and Grace Ngai ([Transformation-Based Learning in the 
Fast Lane](https://www.aclweb.org/anthology/N/N01/N01-1006.pdf)) in devising
some performance enhancements, though. One big enhancement is built in 
support for multithreaded training. If you have 4 processor cores, you can 
set cls.training_threads to 4 and when you call cls.train(), it will keep 
all of them busy and run about 4 times faster. Set cls.use_best_rules to 10 
and training will run about 10 times faster with potentially minimal loss of
accuracy (or increase in number) of training rules devised. I also added 
in many low-level features like early bailing, hashing, and modest caching 
to optimize speed and memory usage.

I purposely decided not to include support for reading and writing files 
because I didn't want to prejudice you, the developer, as to how and where 
you should store your own data.

## Future goals

I believe my code is solid, but I plan to test to see if there are any memory 
leaks or bugs. Please do let me know if you find them.

In the spirit of [fnTLB](https://www.cs.jhu.edu/~rflorian/fntbl/), I might 
create a command line wrapper program at some point, but I'll admit I have 
little desire to right now. That seems like it would take a lot of time to 
do justice to and might mislead developers about how best to integrate this 
library into their own projects.

I wrote this to be completely cross-platform, though I built it 
specifically on OS X. I'd welcome having others build this on Windows, 
Linux or other systems and submitting their work so this can be 
distributed as a stand-alone library for all systems.

## License and credit

This code was written entirely by [Jim Carnicelli](http://JimCarnicelli.com). 
Although I hope someday to earn money using my own creation, I've given this 
project an [MIT license](https://en.wikipedia.org/wiki/MIT_License), meaning
you are free to use it pretty much however you want. That said, I would 
really appreciate it if you'd give credit to my contribution in your derived 
works. I'd also welcome you to drop me a line about how you are using it 
and if you've found interesting ways to improve upon it.

Cheers.
