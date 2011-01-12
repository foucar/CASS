/** list of possible commands for lucassview
 *
 * This file is executed on the startup of lucassview.
 */
{
  //set up the server name//
  gCASSClient->setServer("amo-daq-mon02");

  //set up the port the server runs on//
  gCASSClient->setPort(12321);

  //sync the histograms once//
//  gCASSClient->syncHistograms();

  //tell lucassview to automaticly sync the displayed and not existing histograms,//
  //the parameter is in Hz//
  gCASSClient->autoSync(2.5);

  //tell lucassview that once it retrieved the histogram it should also be updated in the Window//
  //without this one has to click into the histogram to update it//
  gCASSClient->alsoUpdateCanvas(true);

  //with this command on tells the server to reload the .ini file (same as crtl-r on jocassview)//
//  gCASSClient->reloadIni();

  //this command writes all histograms into the rootfile whichs name is given as paramter//
//  gCASSClient->writeRootFile("coolRootFileName.root");
}
