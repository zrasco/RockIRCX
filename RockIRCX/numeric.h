/*
** numeric.h
**
** Contains all numeric RAW's for messages/replies
*/

#ifndef __NUMERIC_H__
#define __NUMERIC_H__

/* Misc defines */
#define STOP_PARSING 1

/* Error messages */
#define ERROR_001			"ERROR :Closing Link: %s[%s] 001 (No Authorization)"
#define ERROR_002			"ERROR :Closing Link: %s[%s] 002 (No IRC clients permitted)"
#define ERROR_003			"ERROR :Closing Link: %s[%s] 003 (No more connections)"
#define ERROR_004			"ERROR :Closing Link: %s[%s] 004 (No new connections permitted)"
#define ERROR_005			"ERROR :Closing Link: %s[%s] 005 (Server shutdown by operator)"
#define ERROR_006			"ERROR :Closing Link: %s[%s] 006 (Reverse DNS lookup failed)"
#define ERROR_007			"ERROR :Closing Link: %s[%s] 007 (Killed by system operator)"
#define ERROR_008			"ERROR :Closing Link: %s[%s] 008 (Input flooding)"
#define ERROR_009			"ERROR :Closing Link: %s[%s] 009 (Output Saturation)"
#define ERROR_010			"ERROR :Closing Link: %s[%s] 010 (Quote exceeded)"
#define ERROR_011			"ERROR :Closing Link: %s[%s] 011 (Ping timeout)"
#define ERROR_012			"ERROR :Closing Link: %s[%s] 012 (Too many connections from your IP)"
#define ERROR_013			"ERROR :Closing Link: %s[%s] 013 (Out of resources)"

/* "Extended" error messages (these cannot be called by Client_SendErrorNumber(), Client_SendError() must be used) */
#define	ERROR_014			"ERROR :Closing Link: %s[%s] 014 (You have been banned: %s)"

/* RAW numerics */
#define		RPL_WELCOME						001
#define		RPL_YOURHOST					002
#define		RPL_CREATED						003
#define		RPL_MYINFO						004

#define		RPL_UMODEIS						221

#define		RPL_LUSERCLIENT					251
#define		RPL_LUSEROP						252
#define		RPL_LUSERUNKNOWN				253
#define		RPL_LUSERCHANNELS				254
#define		RPL_LUSERME						255
#define		RPL_ADMINME						256
#define		RPL_ADMINLOC1					257
#define		RPL_ADMINLOC2					258
#define		RPL_ADMINEMAIL					259
#define		RPL_LOCALUSERS					265
#define		RPL_GLOBALUSERS					266
#define		RPL_SILELIST					271
#define		RPL_ENDOFSILELIST				272

#define		RPL_NONE						300
#define		RPL_AWAY						301
#define		RPL_USERHOST					302
#define		RPL_ISON						303
#define		RPL_UNAWAY						305
#define		RPL_NOWAWAY						306
#define		RPL_WHOISUSER					311
#define		RPL_WHOISSERVER					312
#define		RPL_WHOISOPERATOR				313
#define		RPL_ENDOFWHO					315
#define		RPL_WHOISIDLE					317
#define		RPL_ENDOFWHOIS					318
#define		RPL_WHOISCHANNELS				319
#define		RPL_WHOISSPECIAL				320
#define		RPL_CHANNELMODEIS				324
#define		RPL_NOTOPIC						331
#define		RPL_TOPIC						332
#define		RPL_INVITING					341
#define		RPL_INVITED						345
#define		RPL_VERSION						351
#define		RPL_WHOREPLY					352
#define		RPL_NAMREPLY					353
#define		RPL_ENDOFNAMES					366
#define		RPL_BANLIST						367
#define		RPL_ENDOFBANLIST				368
#define		RPL_MOTD						372
#define		RPL_MOTDSTART					375
#define		RPL_ENDOFMOTD					376
#define		RPL_YOUREOPER					381
#define		RPL_TIME						391

#define		ERR_NOSUCHNICK					401
#define		ERR_NOSUCHCHANNEL				403
#define		ERR_CANNOTSENDTOCHAN			404
#define		ERR_TOOMANYCHANNELS				405
#define		ERR_NORECIPIENT					411
#define		ERR_NOTEXTTOSEND				412
#define		ERR_UNKNOWNCOMMAND				421
#define		ERR_NOMOTD						422
#define		ERR_NOADMININFO					423
#define		ERR_NONICKNAMEGIVEN				431
#define		ERR_ERRONEUSNICKNAME			432
#define		ERR_NICKNAMEINUSE				433
#define		ERR_NICKTOOFAST					438
#define		ERR_USERNOTINCHANNEL			441
#define		ERR_NOTONCHANNEL				442
#define		ERR_USERONCHANNEL				443
#define		ERR_USERSDISABLED				446
#define		ERR_NOTREGISTERED				451
#define		ERR_NEEDMOREPARAMS				461
#define		ERR_ALREADYREGISTRED			462
#define		ERR_KEYSET						467

#define		ERR_CHANNELISFULL				471
#define		ERR_UNKNOWNMODE					472
#define		ERR_INVITEONLYCHAN				473
#define		ERR_BANNEDFROMCHAN				474
#define		ERR_BADCHANNELKEY				475
#define		ERR_BADCHANMASK					476
#define		ERR_NOPRIVILEGES				481
#define		ERR_CHANOPRIVSNEEDED			482
#define		ERR_UNIQOPPRIVSNEEDED			485
#define		ERR_NOOPERHOST					491

#define		ERR_UMODEUNKNOWNFLAG			501
#define		ERR_USERSDONTMATCH				502

#define		IRCRPL_IRCX						800
#define		IRCRPL_ACCESSADD				801
#define		IRCRPL_ACCESSDELETE				802
#define		IRCRPL_ACCESSSTART				803
#define		IRCRPL_ACCESSLIST				804
#define		IRCRPL_ACCESSEND				805
#define		IRCRPL_EVENTADD					806
#define		IRCRPL_EVENTDEL					807
#define		IRCRPL_EVENTSTART				808
#define		IRCRPL_EVENTLIST				809
#define		IRCRPL_EVENTEND					810
#define		IRCRPL_LISTXSTART				811
#define		IRCRPL_LISTXLIST				812
#define		IRCRPL_LISTXPICS				813
#define		IRCRPL_LISTXTRUNC				816
#define		IRCRPL_LISTXEND					817
#define		IRCRPL_PROPLIST					818
#define		IRCRPL_PROPEND					819
#define		IRCERR_BADCOMMAND				900
#define		IRCERR_TOOMANYARGUMENTS			901
#define		IRCERR_BADFUNCTION				902
#define		IRCERR_BADLEVEL					903
#define		IRCERR_BADTAG					904
#define		IRCERR_BADPROPERTY				905
#define		IRCERR_BADVALUE					906
#define		IRCERR_RESOURCE					907
#define		IRCERR_SECURITY					908
#define		IRCERR_ALREADYAUTHENTICATED		909
#define		IRCERR_AUTHENTICATIONFAILED		910
#define		IRCERR_AUTHENTICATIONSUSPENDED	911
#define		IRCERR_UNKNOWNPACKAGE			912
#define		IRCERR_NOACCESS					913
#define		IRCERR_DUPACCESS				914
#define		IRCERR_MISACCESS				915
#define		IRCERR_TOOMANYACCESSES			916
#define		IRCERR_EVENTDUP					918
#define		IRCERR_EVENTMIS					919
#define		IRCERR_NOSUCHEVENT				920
#define		IRCERR_TOOMANYEVENTS			921
#define		IRCERR_ACCESSSECURITY			922
#define		IRCERR_NOWHISPER				923
#define		IRCERR_NOSUCHOBJECT				924
#define		IRCERR_NOTSUPPORTED				925
#define		IRCERR_CHANNELEXIST				926
#define		IRCERR_ALREADYONCHANNEL			927
#define		IRCERR_UNKNOWNERROR				999


#endif		/* __NUMERIC_H__ */