351Project 1
Bitbros: Starbuck Beagley, Oscar Chacon

	Our main approach was to first get a basic send and recieve working between a single client and the server. With them being able to communicate, we knew that we could start bouncing off
actual messages. Once we got messages properly sent, translated, edited by the server, sent back, and properly displayed, we tried to then fix our bigger issues of queueing.
There were annoyances when it came to sending/recieving the messages, but that was mainly due to the fact that we were using C which makes doing such things trickier and more annoying. 
Overall, the harder challenges were to properly format each message and make sure that each client was getting the same message and the queueing for overpopulation. Our utilization of a 
separate chat queue file helped organize the server a lot more than just trying to stick it all in one file, and it made it a lot easier to debug. We debugged by each testing the problem at hand
and seeing if we were both affected or if it was just one of us. A few breaklines, stupid mistakes, and little tweaks later and we were able to get most of it working properly rather quickly.

