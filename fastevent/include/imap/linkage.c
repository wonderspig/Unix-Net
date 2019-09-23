  mail_link (&mboxdriver);		/* link in the mbox driver */
  mail_link (&imapdriver);		/* link in the imap driver */
  mail_link (&nntpdriver);		/* link in the nntp driver */
  mail_link (&pop3driver);		/* link in the pop3 driver */
  mail_link (&mixdriver);		/* link in the mix driver */
  mail_link (&mxdriver);		/* link in the mx driver */
  mail_link (&mbxdriver);		/* link in the mbx driver */
  mail_link (&tenexdriver);		/* link in the tenex driver */
  mail_link (&mtxdriver);		/* link in the mtx driver */
  mail_link (&mhdriver);		/* link in the mh driver */
  mail_link (&mmdfdriver);		/* link in the mmdf driver */
  mail_link (&unixdriver);		/* link in the unix driver */
  mail_link (&newsdriver);		/* link in the news driver */
  mail_link (&philedriver);		/* link in the phile driver */
  mail_link (&dummydriver);		/* link in the dummy driver */
  auth_link (&auth_gss);		/* link in the gss authenticator */
  auth_link (&auth_ext);		/* link in the ext authenticator */
  auth_link (&auth_md5);		/* link in the md5 authenticator */
  auth_link (&auth_pla);		/* link in the pla authenticator */
  auth_link (&auth_log);		/* link in the log authenticator */
  mail_versioncheck (CCLIENTVERSION);
  ssl_onceonlyinit ();
