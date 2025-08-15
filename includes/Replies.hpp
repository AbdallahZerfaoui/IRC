#ifndef REPLIES_HPP
# define REPLIES_HPP

enum ReplyCode {
    // Registration
	RPL_NULL               = 0, // Special code for no reply
    RPL_WELCOME            = 1,

    // TOPIC
    RPL_NOTOPIC            = 331,
    RPL_TOPIC              = 332,
	RPL_HELPSTART		   = 704,
	RPL_HELPTXT			   = 705,
	RPL_ENDOFHELP		   = 706,
	RPL_INVITING		   = 341,

    // Errors for users/channels
    ERR_NOSUCHNICK         = 401,
    ERR_NOSUCHCHANNEL      = 403,
    ERR_CANNOTSENDTOCHAN   = 404,
    ERR_NOORIGIN           = 409,
    ERR_NORECIPIENT        = 411,
    ERR_UNKNOWNCOMMAND     = 421,
    ERR_NONICKNAMEGIVEN    = 431,
    ERR_ERRONEUSNICKNAME   = 432,
    ERR_NICKNAMEINUSE      = 433,
    ERR_USERNOTINCHANNEL   = 441,
    ERR_NOTONCHANNEL       = 442,
    ERR_USERONCHANNEL      = 443,
    ERR_NOTREGISTERED      = 451,
    ERR_NEEDMOREPARAMS     = 461,
    ERR_ALREADYREGISTERED  = 462,
    ERR_PASSWDMISMATCH     = 464,
    ERR_CHANNELISFULL      = 471,
    ERR_UNKNOWNMODE        = 472,
    ERR_INVITEONLYCHAN     = 473,
    ERR_BADCHANNELKEY      = 475,
    ERR_BADCHANMASK        = 476,
    ERR_CHANOPRIVSNEEDED   = 482,
    ERR_UMODEUNKNOWNFLAG   = 501,
};

#endif
