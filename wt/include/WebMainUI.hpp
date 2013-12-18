/*
 * MainWebWindow.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 12-11-2013                                                  #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */


#ifndef MAINWEBWINDOW_HPP
#define MAINWEBWINDOW_HPP

#include "WebDashboard.hpp"
#include "DbSession.hpp"
#include "UserMngtUI.hpp"
#include <Wt/WTimer>
#include <Wt/WApplication>
#include <Wt/Auth/Login>
#include <Wt/WTabWidget>

class WebMainUI : public QObject, public Wt::WApplication
{
  Q_OBJECT
public:
  WebMainUI(const Wt::WEnvironment& env);
  virtual ~WebMainUI();
  void showAdminHome(void);
  void showLoginHome(void);
  QString getConfig (void) const {return m_currentDashboard->config();}
  void enable(void) {m_mainWidget->enable();}
  void disbale(void) {m_mainWidget->disable();}
  void resetTimer(void);
  void handleInternalPath(void);

public Q_SLOTS:
  void resetTimer(qint32 interval);

private:
  enum FileDialogAction {
    OPEN = 1,
    IMPORT = 0
  };

  Settings* m_settings;
  Wt::WTimer* m_timer;
  Wt::WWidget* createNavBar(void);
  Wt::WWidget* createToolBar(void);
  Wt::WContainerWidget* m_mainWidget;
  Wt::WTabWidget* m_dashtabs;
  Wt::WDialog* m_fileUploadDialog;
  Wt::WFileUpload* m_uploader;
  std::string m_selectFile;
  Wt::WText* m_infoBox;
  WebDashboard* m_currentDashboard;
  typedef std::map<std::string, WebDashboard*> DashboardListT;
  DashboardListT m_dashboards;
  DbSession* m_dbSession;
  Wt::Auth::Login m_login; /* slot conflict if decleared in the DbSession class */
  std::string m_confdir;
  UserMngtUI* m_userMgntUI;


  void addEvents(void);
  void handleAuthentification(void);
  Wt::WPushButton* createTooBarButton(const std::string& icon);
  void handleRefresh(void);
  Wt::WAnchor* createLogoLink(void);
  void openFileUploadDialog(void);
  void selectFileToOpen(void);
  void openFile(const std::string& path);
  void finishFileDialog(int action);
  void scaleMap(double factor);
  Wt::WWidget* createAdminHome(void);
  Wt::WWidget* createLoginHome(void);
  Wt::WAnchor* createAnchorForHomeLink(const std::string& title,
                                       const std::string& desc,
                                       const std::string& internalPath);
  void checkUserLogin(void);
  void showUserMngtPage(Wt::WStackedWidget* contents, int destination);
  void logout(void) {m_login.logout();}
};

#endif // MAINWEBWINDOW_HPP
