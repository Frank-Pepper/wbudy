/******************************************************************
 *****                                                        *****
 *****  Name: webside.c                                       *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****        adannenb@et.htwk-leipzig.de                     *****
 *****  Func: example HTML-code for easyweb.c                 *****
 *****                                                        *****
 ******************************************************************/

// Embedded Artists - Example webpage updated

const unsigned char WebSide[] = {
"<html>\r\n"
"<head>\r\n"
"<title>LPC1768 - Demo page</title>\r\n"
"</head>\r\n"
"\r\n"
"<body bgcolor=\"#F5F5F5\" text=\"#336688\">\r\n"
"<p><b><font color=\"black\" size=\"6\"><i>Hello World from Embedded Artists!</i></font></b></p>\r\n"
"\r\n"
"<p><b>This is a website hosted by the embedded Webserver</b> <b>easyWEB.</b></p>\r\n"
"<p><b>Hardware:</b></p>\r\n"
"<ul>\r\n"
"<li><b>LPC1768 LPCXpresso Board - NXP LPC1768 MCU with ARM Cortex-M3 CPU</b></li>\r\n"
"<li><b>Embedded Artists LPCXpresso Base Board</b></li>\r\n"
"<li><b>Embedded EMAC Ethernet Controller</b></li>\r\n"
"</ul>\r\n"
"\r\n"
"</body>\r\n"
"</html>\r\n"
"\r\n"};


// Original page removed
/*
const unsigned char WebSide[] = {
"<html>\r\n"
"<head>\r\n"
"<meta http-equiv=\"refresh\" content=\"5\">\r\n"
"<title>easyWEB - dynamic Webside</title>\r\n"
"</head>\r\n"
"\r\n"
"<body bgcolor=\"#3030A0\" text=\"#FFFF00\">\r\n"
"<p><b><font color=\"#FFFFFF\" size=\"6\"><i>Hello World!</i></font></b></p>\r\n"
"\r\n"
"<p><b>This is a dynamic webside hosted by the embedded Webserver</b> <b>easyWEB.</b></p>\r\n"
"<p><b>Hardware:</b></p>\r\n"
"<ul>\r\n"
"<li><b>MSP430F149, 8 MHz, 60KB Flash, 2KB SRAM</b></li>\r\n"
"<li><b>CS8900A Crystal Ethernet Controller</b></li>\r\n"
"</ul>\r\n"
"\r\n"
"<p><b>A/D Converter Value Port P6.7:</b></p>\r\n"
"\r\n"
"<table bgcolor=\"#ff0000\" border=\"5\" cellpadding=\"0\" cellspacing=\"0\" width=\"500\">\r\n"
"<tr>\r\n"
"<td>\r\n"
"<table width=\"AD7%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\r\n"
"<tr><td bgcolor=\"#00ff00\">&nbsp;</td></tr>\r\n"
"</table>\r\n"
"</td>\r\n"
"</tr>\r\n"
"</table>\r\n"
"\r\n"
"<table border=\"0\" width=\"500\">\r\n"
"<tr>\r\n"
"<td width=\"20%\">0V</td>\r\n"
"<td width=\"20%\">0,5V</td>\r\n"
"<td width=\"20%\">1V</td>\r\n"
"<td width=\"20%\">1,5V</td>\r\n"
"<td width=\"20%\">2V</td>\r\n"
"</tr>\r\n"
"</table>\r\n"
"\r\n"
"<p><b>MCU Temperature:</b></p>\r\n"
"\r\n"
"<table bgcolor=\"#ff0000\" border=\"5\" cellpadding=\"0\" cellspacing=\"0\" width=\"500\">\r\n"
"<tr>\r\n"
"<td>\r\n"
"<table width=\"ADA%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\r\n"
"<tr><td bgcolor=\"#00ff00\">&nbsp;</td></tr> \r\n"
"</table>\r\n"
"</td>\r\n"
"</tr>\r\n"
"</table>\r\n"
"\r\n"
"<table border=\"0\" width=\"500\">\r\n"
"<tr>\r\n"
"<td width=\"20%\">20�C</td>\r\n"
"<td width=\"20%\">25�C</td>\r\n"
"<td width=\"20%\">30�C</td>\r\n"
"<td width=\"20%\">35�C</td>\r\n"
"<td width=\"20%\">40�C</td>\r\n"
"</tr>\r\n"
"</table>\r\n"
"</body>\r\n"
"</html>\r\n"
"\r\n"};
*/
