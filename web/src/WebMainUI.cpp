﻿/*
 * MainWebWindow.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 23-03-2014                                                  #
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

#include "dbo/src/ViewAclManagement.hpp"
#include "AuthManager.hpp"
#include "WebMainUI.hpp"
#include "utilsCore.hpp"
#include "WebUtils.hpp"
#include "WebInputSelector.hpp"
#include <Wt/WApplication>
#include <Wt/WToolBar>
#include <Wt/WPushButton>
#include <Wt/WPopupMenu>
#include <functional>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>
#include <Wt/WImage>
#include <Wt/WProgressBar>
#include <Wt/WDialog>
#include <Wt/WSelectionBox>
#include <Wt/WTemplate>
#include <Wt/WHBoxLayout>
#include <Wt/WEvent>

#define RESIZE_PANES \
  "var top = $(\"#ngrt4n-content-pane\").offset().top;" \
  "var windowHeight = $(window).height() - 40;" \
  "$(\"#ngrt4n-content-pane\").height(windowHeight - top);" \
  "$(\"#ngrt4n-side-pane\").height(windowHeight - top);"


WebMainUI::WebMainUI(AuthManager* authManager)
  : Wt::WContainerWidget(),
    sessionTerminated(this),
    m_rootDir("/opt/realopinsight"),
    m_configDir(m_rootDir.append("/data")),
    m_authManager(authManager),
    m_dbSession(m_authManager->session()),
    m_currentDashboard(nullptr),
    m_fileUploader(nullptr),
    m_thumbsLayout(nullptr),
    m_notificationManager(nullptr),
    m_notificationSection(nullptr),
    m_eventFeedLayout(nullptr)
{
  m_dataSourceSettings.setDbSession(m_dbSession);
  // export configuration environment variables
  qputenv("REALOPINSIGHT_ROOT_DIR", m_rootDir.toUtf8());
  qputenv("REALOPINSIGHT_CONFIG_DIR", m_configDir.toUtf8());

  m_breadcrumbsBar = createBreadCrumbsBarTpl();
  m_userAccountForm = createAccountPanel();
  m_changePasswordPanel = createPasswordPanel();
  m_aboutDialog = createAboutDialog();

  m_webEditor.setConfigDir(m_configDir);
  m_webEditor.setDbSession(m_dbSession);
  m_webEditor.refreshDynamicContents();

  addWidget(&m_mainWidget);
  setupInfoBox();
  setupMainUI();
  configureDialogWidget();

  setupMenus();
  showUserHome();
  setupUploadForm();
  doJavaScript(RESIZE_PANES);
  addEvents();


  m_infoBox.setId("info-box");
  m_mainWidget.setId("maincontainer");
  m_mainStackedContents.setId("stackcontentarea");
  m_dashboardStackedContents.addStyleClass("wrapper-container");
}

WebMainUI::~WebMainUI()
{
  unbindWidgets();
  CORE_LOG("debug", "Session closed");
}



void WebMainUI::unbindWidgets(void)
{
  unbindDashboardWidgets();
  unbindExecutiveViewWidgets();
  unbindStackedWidgets();

  m_adminStackedContents.removeWidget(&m_dataSourceSettings);
  m_adminStackedContents.removeWidget(&m_notificationSettingsForm);
  m_adminStackedContents.removeWidget(&m_authSettingsForm);
  m_adminStackedContents.removeWidget(&m_webEditor);
  m_adminStackedContents.removeWidget(&m_adminHomePageTpl);
  m_adminStackedContents.removeWidget(&m_databaseSettingsForm);

  m_uploadForm.contents()->removeWidget(&m_uploadFormTitle);
  m_uploadForm.contents()->removeWidget(&m_uploadSubmitButton);
  m_uploadForm.contents()->removeWidget(&m_uploadCancelButton);

  m_settingsMainPageTpl.takeWidget("title");
  m_settingsMainPageTpl.takeWidget("contents");
  m_settingsMainPageTpl.takeWidget("info-box");
  m_settingsMainPageTpl.clear();

  m_breadcrumbsBar->clear();
  m_mainWidget.removeWidget(&m_navbar);
  m_mainWidget.removeWidget(&m_mainStackedContents);
  removeWidget(&m_mainWidget);
  clear();
}



void WebMainUI::unbindExecutiveViewWidgets(void)
{
  for (ThumbnailMapT::Iterator thumb = m_thumbsWidgets.begin(); thumb != m_thumbsWidgets.end(); ++thumb) {
    clearThumbnailTemplate(*thumb);
    m_thumbsLayout->removeWidget(*thumb);
  }

  if (m_thumbsLayout && m_thumbsLayout->children().size() > 0) {
    m_thumbsContainer.clear();
  }
  if (m_eventFeedLayout && m_eventFeedLayout->children().size() > 0) {
    m_eventFeedsContainer.clear();
  }

  m_operatorHomeTpl.takeWidget("thumbnails");
  m_operatorHomeTpl.takeWidget("event-feeds");
  m_operatorHomeTpl.takeWidget("bi-report-dashlet");
  m_operatorHomeTpl.takeWidget("info-box");
}


void WebMainUI::unbindDashboardWidgets(void)
{
  for (auto dashboardItem : m_dashboardMap) {
    if (m_eventFeedLayout) {
      m_eventFeedLayout->removeItem(dashboardItem->eventFeedLayout());
    }
    m_dashboardStackedContents.removeWidget(dashboardItem);
  }
}


void WebMainUI::unbindStackedWidgets(void)
{
  m_dashboardStackedContents.removeWidget(&m_settingsMainPageTpl);
  m_dashboardStackedContents.removeWidget(&m_operatorHomeTpl);
  m_mainStackedContents.removeWidget(&m_dashboardStackedContents);
}

void WebMainUI::addEvents(void)
{
  QObject::connect(&m_biDashlet, SIGNAL(reportPeriodChanged(long,long)), this, SLOT(handleReportPeriodChanged(long, long)));
  m_dataSourceSettings.operationCompleted().connect(this, &WebMainUI::showMessage);
  m_authSettingsForm.operationCompleted().connect(this, &WebMainUI::showMessage);
  m_databaseSettingsForm.operationCompleted().connect(this, &WebMainUI::showMessage);
  m_authSettingsForm.authSystemChanged().connect(this, &WebMainUI::handleAuthSystemChanged);
  m_globalTimer.timeout().connect(this, &WebMainUI::handleRefresh);
  m_uploadSubmitButton.clicked().connect(this, &WebMainUI::handleUploadSubmitButton);
  m_uploadCancelButton.clicked().connect(this, &WebMainUI::handleUploadCancelButton);
}


void WebMainUI::showUserHome(void)
{
  std::string homeTabTitle = "Home";
  if (m_dbSession->isLoggedAdmin()) {
    homeTabTitle = tr("%1 - Administration").arg(APP_NAME).toStdString();
  } else {
    homeTabTitle =  tr("%1 - Operations Console").arg(APP_NAME).toStdString();
  }
  std::string pageTitle = homeTabTitle;
  pageTitle.append(" - ").append(m_dbSession->loggedUser().username);
  wApp->setTitle(pageTitle);
  setupSettingsPage();
  m_dashboardStackedContents.addWidget(&m_settingsMainPageTpl);
  if (! m_dbSession->isLoggedAdmin()) {
    initOperatorDashboard();
    m_dashboardStackedContents.setCurrentWidget(&m_operatorHomeTpl);
  }
}

void WebMainUI::setupMainUI(void)
{
  setupNavivationBar();
  setupMainStackedContent();
  m_mainWidget.addWidget(&m_navbar);
  m_mainWidget.addWidget(&m_mainStackedContents);
}


void WebMainUI::setupNavivationBar(void)
{
  m_navbar.setResponsive(true);
  m_navbar.addWidget(createLogoLink(), Wt::AlignLeft);
  m_navbar.addWidget(new Wt::WTemplate(Wt::WString::tr("beta-message")));
}


Wt::WTemplate* WebMainUI::createBreadCrumbsBarTpl(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("breadcrumbs-bar.tpl"));


  m_selectViewBox = new Wt::WComboBox();
  if (! m_dbSession->isLoggedAdmin()) {
    m_selectViewBox->addItem(Q_TR("Executive View"));
  } else {
    m_selectViewBox->addItem(Q_TR("Admin Home"));
  }
  m_selectViewBox->changed().connect(this, &WebMainUI::handleNewViewSelected);
  tpl->bindWidget("display-view-selection-box", m_selectViewBox);


  m_showOnlyProblemMsgsField = new Wt::WCheckBox(Q_TR("Show only problems"));
  m_showOnlyProblemMsgsField->changed().connect(this, &WebMainUI::handleShowOnlyProblemsMsgs);
  m_showOnlyProblemMsgsField->setHidden(true);
  tpl->bindWidget("display-only-trouble-event-box", m_showOnlyProblemMsgsField);

  return tpl;
}

void WebMainUI::setupMainStackedContent(void)
{
  m_mainStackedContents.addWidget(&m_dashboardStackedContents);
}



void WebMainUI::handleShowSettingsView(void)
{
  if (! m_dbSession->isLoggedAdmin()) {
    hideAdminSettingsMenu();
  }
  swicthFrontStackedWidgetTo(&m_settingsMainPageTpl);
}

void WebMainUI::handleShowExecutiveView(void)
{
  swicthFrontStackedWidgetTo(&m_operatorHomeTpl);
  resetViewSelectionBox();
}


void WebMainUI::setupInfoBox(void)
{
  m_infoBox.hide();
  m_infoBox.clicked().connect(&m_infoBox, &Wt::WText::hide);
}

void WebMainUI::setupProfileMenus(void)
{
  Wt::WMenu* profileMenu = new Wt::WMenu();
  m_navbar.addMenu(profileMenu, Wt::AlignRight);
  
  if (! m_dbSession->isLoggedAdmin()) {
    m_notificationManager = createNotificationManager();
    m_notificationSection = createNotificationSection();
    m_notificationSection->setToolTip(Q_TR("Manage notification settings"));
    m_navbar.addWidget(m_notificationSection, Wt::AlignRight);
  }
  
  Wt::WMenuItem* profileMenuItem = new Wt::WMenuItem(tr("Signed in as %1").arg(m_dbSession->loggedUserName()).toStdString());
  Wt::WPopupMenu* profilePopupMenu = new Wt::WPopupMenu();
  profileMenuItem->setMenu(profilePopupMenu);
  profileMenu->addItem(profileMenuItem);

  // link Settings
  profilePopupMenu->addItem(tr("Settings").toStdString())
      ->clicked().connect(this, &WebMainUI::handleShowSettingsView);

  // link Help
  Wt::WMenuItem* currentMenuItem = profilePopupMenu->addItem(tr("Help").toStdString());
  currentMenuItem->setLink(Wt::WLink(Wt::WLink::Url, REALOPINSIGHT_GET_HELP_URL));
  currentMenuItem->setLinkTarget(Wt::TargetNewWindow);

  // link About
  profilePopupMenu->addItem("About")
      ->triggered().connect(m_aboutDialog, &Wt::WDialog::show);
}

Wt::WWidget* WebMainUI::createNotificationSection(void)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("notification.block.tpl"));

  m_notificationBoxes[ngrt4n::Normal] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Normal]->setStyleClass("btn btn-normal");
  m_notificationBoxes[ngrt4n::Normal]->setHidden(true);
  tpl->bindWidget("normal-count", m_notificationBoxes[ngrt4n::Normal]);

  m_notificationBoxes[ngrt4n::Minor] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Minor]->setStyleClass("btn btn-minor");
  m_notificationBoxes[ngrt4n::Minor]->setHidden(true);
  tpl->bindWidget("minor-count", m_notificationBoxes[ngrt4n::Minor]);

  m_notificationBoxes[ngrt4n::Major] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Major]->setStyleClass("btn btn-major");
  m_notificationBoxes[ngrt4n::Major]->setHidden(true);
  tpl->bindWidget("major-count", m_notificationBoxes[ngrt4n::Major]);

  m_notificationBoxes[ngrt4n::Critical] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Critical]->setStyleClass("btn btn-critical");
  m_notificationBoxes[ngrt4n::Critical]->setHidden(true);
  tpl->bindWidget("critical-count", m_notificationBoxes[ngrt4n::Critical]);

  m_notificationBoxes[ngrt4n::Unknown] = new Wt::WText("0");
  m_notificationBoxes[ngrt4n::Unknown]->setStyleClass("btn btn-unknown");
  m_notificationBoxes[ngrt4n::Unknown]->setHidden(true);
  tpl->bindWidget("unknown-count", m_notificationBoxes[ngrt4n::Unknown]);

  tpl->clicked().connect(this, &WebMainUI::handleShowNotificationManager);

  return tpl;
}



void WebMainUI::hideAdminSettingsMenu(void)
{
  m_notificationSettingsForm.setHidden(true);
  m_authSettingsForm.setHidden(true);
  wApp->doJavaScript("$('#userMenuBlock').hide();"
                     "$('#viewMenuBlock').hide();"
                     "$('#menu-auth-settings').hide();"
                     "$('#menu-notification-settings').hide();"
                     "$('#menu-license-activation').hide();");
}



void WebMainUI::setupMenus(void)
{
  setupProfileMenus();
  
  //FIXME: add this after the first view loaded
  Wt::WText* text = ngrt4n::createFontAwesomeTextButton("fa fa-refresh", "Refresh the console map");
  text->clicked().connect(this, &WebMainUI::handleRefresh);
  m_navbar.addWidget(text);
  
  text = ngrt4n::createFontAwesomeTextButton("fa fa-search-plus", "Zoom the console map in");
  text->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALIN_FACTOR));
  m_navbar.addWidget(text);
  
  text = ngrt4n::createFontAwesomeTextButton("fa fa-search-minus","Zoom the console map out");
  text->clicked().connect(std::bind(&WebMainUI::scaleMap, this, ngrt4n::SCALOUT_FACTOR));
  m_navbar.addWidget(text);
  // m_navbar will take the ownership on m_breadcrumbsBar
  m_navbar.addWidget(m_breadcrumbsBar);
}

void WebMainUI::startTimer(void)
{
  m_globalTimer.setInterval(1000 * m_settings.updateInterval());
  m_globalTimer.start();
}

void WebMainUI::resetTimer(qint32 interval)
{
  m_globalTimer.stop();
  m_globalTimer.setInterval(interval);
  m_globalTimer.start();
}



void WebMainUI::handleRefresh(void)
{
  CORE_LOG("info", QObject::tr("updating console (operator: %1, session: %2)").arg(m_dbSession->loggedUserName(), wApp->sessionId().c_str()).toStdString());
  m_globalTimer.stop();

  std::map<int, int> problemTypeCount;
  problemTypeCount[ngrt4n::Normal]   = 0;
  problemTypeCount[ngrt4n::Minor]    = 0;
  problemTypeCount[ngrt4n::Major]    = 0;
  problemTypeCount[ngrt4n::Critical] = 0;
  problemTypeCount[ngrt4n::Unknown]  = 0;

  // clear the notification manager when applicable
  if (m_notificationManager) {
    m_notificationManager->clearAllServicesData();
  }

  QosDataListMapT qosDataMap;
  m_dbSession->listQosData(qosDataMap,"" /* empty view name means all views */, m_biDashlet.startTime(), m_biDashlet.endTime());
  int currentView = 1;
  for (auto& dashboard : m_dashboardMap) {
    dashboard->setDbSession(m_dbSession);
    auto loadDsOut = dashboard->loadDataSources();
    if (loadDsOut.first != ngrt4n::RcSuccess) {
      CORE_LOG("error", loadDsOut.second.toStdString());
      continue;
    }
    dashboard->updateAllNodesStatus();
    dashboard->updateMap();
    dashboard->updateThumbnailInfo();
    NodeT currentRootNode = dashboard->rootNode();
    int platformSeverity = qMin(currentRootNode.sev, static_cast<int>(ngrt4n::Unknown));
    if (platformSeverity != ngrt4n::Normal) {
      ++problemTypeCount[platformSeverity];
      if (m_notificationManager) {
        m_notificationManager->updateServiceData(currentRootNode);
      }
    }
    std::string viewName = currentRootNode.name.toStdString();
    ThumbnailMapT::Iterator thumbnailItem = m_thumbsWidgets.find(viewName);
    if (thumbnailItem != m_thumbsWidgets.end()) {
      (*thumbnailItem)->setStyleClass(dashboard->thumbnailCssClass());
      (*thumbnailItem)->setToolTip(dashboard->tooltip());
      if (m_dbSession->isCompleteUserDashboard()) {
        m_biDashlet.updateChartsByViewName(viewName, qosDataMap);
      }
    }
    ++currentView;
  }

  // Display notifications only on operator console
  if (! m_dbSession->isLoggedAdmin()) {
    for (auto severityEntry: problemTypeCount) {
      m_notificationBoxes[severityEntry.first]->setText(QString::number(severityEntry.second).toStdString());
      m_notificationBoxes[severityEntry.first]->setHidden(severityEntry.second <= 0);
    }
  }

  startTimer();

  CORE_LOG("info", QObject::tr("console update completed (operator: %1, session: %2)").arg(m_dbSession->loggedUserName(), wApp->sessionId().c_str()).toStdString());
}



void WebMainUI::handleLaunchEditor(void)
{
  m_adminPanelTitle.setText(Q_TR("Service Editor"));
  m_adminStackedContents.setCurrentWidget(&m_webEditor);
  m_webEditor.refreshDynamicContents();
}



void WebMainUI::handlePreviewFile(const std::string& path, const std::string&)
{
  auto loadViewOut = loadView(path);
  if (loadViewOut.first) {
    loadViewOut.first->setShowOnlyProblemMsgsState(m_showOnlyProblemMsgsField->checkState()==Wt::Checked);
    setDashboardAsFrontStackedWidget(loadViewOut.first);
  } else {
    showMessage(ngrt4n::OperationSucceeded, loadViewOut.second.toStdString());
  }
}


void WebMainUI::handleAuthSettings(void)
{
  m_adminPanelTitle.setText(Q_TR("Authentication Settings"));
  m_adminStackedContents.setCurrentWidget(&m_authSettingsForm);
  m_authSettingsForm.updateContents();
}


void WebMainUI::handleNotificationSettings(void)
{
  m_adminPanelTitle.setText(Q_TR("Notification Settings"));
  m_adminStackedContents.setCurrentWidget(&m_notificationSettingsForm);
  m_notificationSettingsForm.updateContents();
}


void WebMainUI::handleDatabaseSettings(void)
{
  m_adminPanelTitle.setText(Q_TR("Database Settings"));
  m_adminStackedContents.setCurrentWidget(&m_databaseSettingsForm);
  m_databaseSettingsForm.updateContents();
}


void WebMainUI::handleUserProfileSettings(void)
{
  m_userAccountForm->resetValidationState(false);
  m_adminStackedContents.setCurrentWidget(m_userAccountForm);
  m_adminPanelTitle.setText(Q_TR("My Account"));
}


void WebMainUI::handleUpdateUserAccount(const DboUserT& userToUpdate)
{
  handleErrcode(m_dbSession->updateUser(userToUpdate).first);
}


void WebMainUI::handleDisplayChangePassword(void)
{
  m_adminStackedContents.setCurrentWidget(m_changePasswordPanel);
  m_changePasswordPanel->reset();
  m_adminPanelTitle.setText("Change password");
}


void WebMainUI::handleChangePassword(const std::string& login, const std::string& lastpass, const std::string& pass)
{
  handleErrcode(m_dbSession->updatePassword(login, lastpass, pass).first);
}



void WebMainUI::handleReportPeriodChanged(long start, long end)
{
  QosDataListMapT qosDataMap;
  m_dbSession->listQosData(qosDataMap, "" /** empty view name means all views **/, start, end);
  for (const auto& viewName : qosDataMap.keys()) {
    m_biDashlet.updateChartsByViewName(viewName, qosDataMap);
  }
  showMessage(ngrt4n::OperationSucceeded, Q_TR("Reports updated: ")
              .append(ngrt4n::wHumanTimeText(start).toUTF8())
              .append(" - ")
              .append(ngrt4n::wHumanTimeText(end).toUTF8()));
}

void WebMainUI::handleDataSourceSettings(void)
{
  m_adminPanelTitle.setText(Q_TR("Monitoring Sources"));
  m_adminStackedContents.setCurrentWidget(&m_dataSourceSettings);
  m_dataSourceSettings.updateContents();
}


void WebMainUI::handleNewViewSelected(void)
{
  std::string selectedEntry = m_selectViewBox->currentText().toUTF8();
  auto dashboardIter = m_dashboardMap.find(selectedEntry.c_str());
  if (dashboardIter != m_dashboardMap.end()) {
    setDashboardAsFrontStackedWidget(*dashboardIter);
    m_showOnlyProblemMsgsField->setHidden(false);
  } else {
    m_currentDashboard = nullptr;
    m_showOnlyProblemMsgsField->setHidden(true);
    if (! m_dbSession->isLoggedAdmin()) {
      swicthFrontStackedWidgetTo(&m_operatorHomeTpl);
    } else {
      swicthFrontStackedWidgetTo(&m_settingsMainPageTpl);
    }
  }
}


void WebMainUI::handleShowOnlyProblemsMsgs(void)
{
  if (m_showOnlyProblemMsgsField && m_currentDashboard) {
    m_currentDashboard->setShowOnlyProblemMsgsState(m_showOnlyProblemMsgsField->checkState()==Wt::Checked);
    m_currentDashboard->refreshMsgConsoleOnProblemStates();
  }
}

void WebMainUI::handleUploadSubmitButton(void)
{
  m_uploadSubmitButton.disable();
  m_uploadForm.accept();
  m_fileUploader->upload();
  m_uploadSubmitButton.enable();
}



void WebMainUI::handleUploadCancelButton(void)
{
  m_uploadSubmitButton.enable();
  m_uploadForm.accept();
}



void WebMainUI::handleUploadFileTooLarge(void)
{
  WebMainUI::showMessage(ngrt4n::OperationFailed, Q_TR("File too large."));
}



void WebMainUI::handleShowUploadForm(void)
{
  resetFileUploader();
  m_uploadForm.show();
}



Wt::WAnchor* WebMainUI::createLogoLink(void)
{
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink(PKG_URL.toStdString()),
                                        new Wt::WImage("images/built-in/logo-mini.png"));
  anchor->setTarget(Wt::TargetNewWindow);
  return anchor;
}


void WebMainUI::selectItem4Preview(void)
{
  m_previewSelectorDialog.updateContentWithViewList(m_dbSession->listViews());
  m_previewSelectorDialog.show(); // accept action triggers the method handlePreviewFile
}


void WebMainUI::setupUploadForm(void)
{
  m_fileUploader = new Wt::WFileUpload();
  m_fileUploader->setFileTextSize(MAX_FILE_UPLOAD);
  m_fileUploader->setProgressBar(new Wt::WProgressBar()); // takes the ownership of the progressbar
  m_fileUploader->setMargin(10, Wt::Right);

  m_uploadFormTitle.setText(Q_TR("<p>Select a service file</p>"));
  m_uploadFormTitle.setTextFormat(Wt::XHTMLText);

  m_uploadSubmitButton.setText(Q_TR("Upload"));
  m_uploadCancelButton.setText(Q_TR("Close"));

  m_uploadForm.contents()->addWidget(&m_uploadFormTitle);
  m_uploadForm.contents()->addWidget(m_fileUploader);
  m_uploadForm.contents()->addWidget(&m_uploadSubmitButton);
  m_uploadForm.contents()->addWidget(&m_uploadCancelButton);

  m_uploadForm.setStyleClass("Wt-dialog");
  m_uploadForm.setWindowTitle(tr("Importation | %1").arg(APP_NAME).toStdString());
}


std::pair<WebDashboard*, QString>
WebMainUI::loadView(const std::string& path)
{
  if (path.empty()) {
    return {nullptr, QObject::tr("Cannot open empty path")};
  }

  WebDashboard* dashboard = nullptr;
  try {
    dashboard = new WebDashboard(m_dbSession);
    if (! dashboard) {
      return {nullptr, QObject::tr("Cannot allocate the dashboard widget")};
    }

    auto outInitDashboard = dashboard->initialize(&m_dataSourceSettings, path.c_str());
    if (outInitDashboard.first != ngrt4n::RcSuccess) {
      delete dashboard;
      return {nullptr, outInitDashboard.second};
    }

    QString viewName = dashboard->rootNode().name;
    auto loadedDashboardItem = m_dashboardMap.find(viewName);
    if (loadedDashboardItem != m_dashboardMap.end()) {
      // cleanup the existing dashboard before to reload it later
      m_dashboardStackedContents.removeWidget(*loadedDashboardItem);
      m_dashboardMap.remove(viewName);
      delete *loadedDashboardItem;
    }

    m_dashboardMap.insert(viewName, dashboard);
    m_dashboardStackedContents.addWidget(dashboard);
    m_selectViewBox->addItem(viewName.toStdString());

    // the inner layout is explicitely removed when the object is destroyed
    if (m_eventFeedLayout) {
      m_eventFeedLayout->addItem(dashboard->eventFeedLayout());
    }

  } catch (const std::bad_alloc&) {
    std::string errorMsg = tr("Dashboard initialization failed with bad_alloc").toStdString();
    CORE_LOG("error", errorMsg);
    showMessage(ngrt4n::OperationFailed, errorMsg);
    return {nullptr, QObject::tr("Dashboard initialization failed with bad_alloc")};
  }
  return {dashboard, ""};
}

void WebMainUI::scaleMap(double factor)
{
  if (m_currentDashboard) {
    m_currentDashboard->map()->scaleMap(factor);
  }
}

void WebMainUI::setupSettingsPage(void)
{
  m_settingsMainPageTpl.setTemplateText(Wt::WString::tr("admin-home.tpl"));
  m_settingsMainPageTpl.bindWidget("title", &m_adminPanelTitle);
  m_settingsMainPageTpl.bindWidget("contents", &m_adminStackedContents);

  Wt::WAnchor* link = nullptr;
  switch (m_dbSession->loggedUser().role) {
    case DboUser::AdmRole: {
      m_settingsMainPageTpl.bindWidget("info-box", &m_infoBox);
      m_dataSourceSettings.setEnabledInputs(true);

      // Start menu
      std::string menuText = QObject::tr("Welcome").toStdString();
      link = new Wt::WAnchor("#", menuText, &m_mainWidget);
      m_settingsMainPageTpl.bindWidget("menu-get-started", link);
      m_adminHomePageTpl.setTemplateText(Wt::WString::tr("getting-started.tpl"));
      m_adminStackedContents.addWidget(&m_adminHomePageTpl);
      link->clicked().connect(this, &WebMainUI::handleShowAdminHome);
      m_menuLinks.insert(MenuWelcome, link);

      // menu editor
      m_adminStackedContents.addWidget(&m_webEditor);
      m_webEditor.operationCompleted().connect(this, &WebMainUI::showMessage);
      menuText = QObject::tr("Service Editor").toStdString();
      link = new Wt::WAnchor("#", menuText, &m_mainWidget);
      link->clicked().connect(this, &WebMainUI::handleLaunchEditor);
      m_settingsMainPageTpl.bindWidget("menu-editor", link);
      m_menuLinks.insert(MenuEditor, link);

      // menu import view
      menuText = QObject::tr("Import Service File").toStdString();
      link = new Wt::WAnchor("#", menuText, &m_mainWidget);
      link->clicked().connect(this, &WebMainUI::handleShowUploadForm);
      m_settingsMainPageTpl.bindWidget("menu-import-service-file", link);
      m_menuLinks.insert(MenuImport, link);

      // menu preview

      menuText = QObject::tr("Preview").toStdString();
      link = new Wt::WAnchor("#", menuText, &m_mainWidget);
      link->clicked().connect(this, &WebMainUI::selectItem4Preview);
      m_settingsMainPageTpl.bindWidget("menu-preview", link);
      m_menuLinks.insert(MenuPreview, link);

      // Create view management form
      menuText = QObject::tr("Manage Operations Views").toStdString();
      m_viewAccessPermissionForm = new ViewAclManagement(m_dbSession);
      m_adminStackedContents.addWidget(m_viewAccessPermissionForm);
      m_viewAccessPermissionForm->viewDeleted().connect(this, &WebMainUI::handleDeleteView);


      // link views and acl
      link = new Wt::WAnchor("#", menuText);
      link->clicked().connect(this, &WebMainUI::handleViewAclMenu);
      m_settingsMainPageTpl.bindWidget("menu-all-views", link);
      m_menuLinks.insert(MenuViewAndAcl, link);

      // User menus
      m_dbUserManager = new DbUserManager(m_dbSession);
      m_adminStackedContents.addWidget(m_dbUserManager->userForm());
      m_dbUserManager->updateCompleted().connect(this, &WebMainUI::handleUserUpdatedCompleted);

      // link new user
      link = new Wt::WAnchor("#", Q_TR("New User"));
      link->clicked().connect(this, &WebMainUI::handleNewUserMenu);
      m_settingsMainPageTpl.bindWidget("menu-new-user", link);
      m_menuLinks.insert(MenuNewUser, link);

      // built-in menu
      m_adminStackedContents.addWidget(m_dbUserManager->dbUserListWidget());
      link = new Wt::WAnchor("#", Q_TR("All Users"));
      link->clicked().connect(this, &WebMainUI::handleBuiltInUsersMenu);
      m_settingsMainPageTpl.bindWidget("menu-builin-users", link);
      m_menuLinks.insert(MenuBuiltInUsers, link);


      // ldap user menu
      m_ldapUserManager = new LdapUserManager(m_dbSession);
      m_adminStackedContents.addWidget(m_ldapUserManager);
      link = new Wt::WAnchor("#", Q_TR("LDAP Users"));
      link->clicked().connect(this, &WebMainUI::handleLdapUsersMenu);
      m_ldapUserManager->userEnableStatusChanged().connect(this, &WebMainUI::handleUserEnableStatusChanged);
      m_settingsMainPageTpl.bindWidget("menu-ldap-users", link);
      m_menuLinks.insert(MenuLdapUsers, link);
    }
      break;
    default: {
      m_settingsMainPageTpl.bindEmpty("info-box");
      wApp->doJavaScript("$('#userMenuBlock').hide(); $('#viewMenuBlock').hide();");
      m_settingsMainPageTpl.bindEmpty("menu-get-started");
      m_settingsMainPageTpl.bindEmpty("menu-import");
      m_settingsMainPageTpl.bindEmpty("menu-preview");
      m_settingsMainPageTpl.bindEmpty("menu-all-views");
      m_settingsMainPageTpl.bindEmpty("menu-new-user");
      m_settingsMainPageTpl.bindEmpty("menu-all-users");
      m_settingsMainPageTpl.bindEmpty("menu-notification-settings");
    }
      break;
  }

  // monitoring settings menu
  m_adminStackedContents.addWidget(&m_dataSourceSettings);
  link = new Wt::WAnchor("#", Q_TR("Monitoring Sources"));
  m_settingsMainPageTpl.bindWidget("menu-monitoring-settings", link);
  m_menuLinks.insert(MenuMonitoringSettings, link);
  link->clicked().connect(this, &WebMainUI::handleDataSourceSettings);

  // auth settings menu
  m_adminStackedContents.addWidget(&m_authSettingsForm);
  link = new Wt::WAnchor("#", Q_TR("Authentication"));
  m_settingsMainPageTpl.bindWidget("menu-auth-settings", link);
  m_menuLinks.insert(MenuAuthSettings, link);
  link->clicked().connect(this, &WebMainUI::handleAuthSettings);

  // notification settings menu
  m_adminStackedContents.addWidget(&m_notificationSettingsForm);
  link = new Wt::WAnchor("#", Q_TR("Notification"));
  m_settingsMainPageTpl.bindWidget("menu-notification-settings", link);
  m_menuLinks.insert(MenuNotificationSettings, link);
  link->clicked().connect(this, &WebMainUI::handleNotificationSettings);

  // Database settings menu
  m_adminStackedContents.addWidget(&m_databaseSettingsForm);
  link = new Wt::WAnchor("#", Q_TR("Database Backend"));
  m_settingsMainPageTpl.bindWidget("menu-database-settings", link);
  m_menuLinks.insert(MenuDatabaseSettings, link);
  link->clicked().connect(this, &WebMainUI::handleDatabaseSettings);

  // my account menu
  m_adminStackedContents.addWidget(m_userAccountForm);
  link = new Wt::WAnchor("#", Q_TR("My Account"));
  m_settingsMainPageTpl.bindWidget("menu-my-account", link);
  m_menuLinks.insert(MenuMyAccount, link);
  link->clicked().connect(this, &WebMainUI::handleUserProfileSettings);

  // change password settings
  m_adminStackedContents.addWidget(m_changePasswordPanel);
  link = new Wt::WAnchor("#", "Change Password");
  m_settingsMainPageTpl.bindWidget("menu-change-password", link);
  m_menuLinks.insert(MenuChangePassword, link);
  link->clicked().connect(this, &WebMainUI::handleDisplayChangePassword);
}



UserFormView* WebMainUI::createAccountPanel(void)
{
  bool changedPassword(false);
  bool isUserForm(true);
  DboUserT userInfo = m_dbSession->loggedUser().data();
  UserFormView* userAccountForm = new UserFormView(&userInfo, changedPassword, isUserForm);
  userAccountForm->validated().connect(this, &WebMainUI::handleUpdateUserAccount);

  return userAccountForm;
}

UserFormView* WebMainUI::createPasswordPanel(void)
{
  bool changedPassword(true);
  bool userForm(true);
  DboUserT userInfo = m_dbSession->loggedUser().data();
  UserFormView* changePasswordPanel = new UserFormView(&userInfo, changedPassword, userForm);
  changePasswordPanel->changePasswordTriggered().connect(this, &WebMainUI::handleChangePassword);
  return changePasswordPanel;
}



void WebMainUI::handleErrcode(int errcode)
{
  if (errcode != ngrt4n::RcSuccess) {
    showMessage(ngrt4n::OperationFailed, Q_TR("Updated failed"));
  } else {
    showMessage(ngrt4n::OperationSucceeded, Q_TR("Updated successfully"));
  }
}



void WebMainUI::handleUserUpdatedCompleted(int errcode)
{
  if (errcode != 0) {
    showMessage(ngrt4n::OperationFailed, "update failed: check log for details");
  } else {
    showMessage(ngrt4n::OperationSucceeded, Q_TR("Updated successfully"));
    m_dbUserManager->resetUserForm();
  }
}

void WebMainUI::handleShowAdminHome(void)
{
  m_adminStackedContents.setCurrentWidget(&m_adminHomePageTpl);
  m_adminPanelTitle.setText(Q_TR("Getting Started in 3 Simple Steps !"));
}




void WebMainUI::showMessage(int status, const std::string& msg)
{
  std::string logLevel = "debug";
  std::string cssClass = "alert alert-success";
  bool hidden = false;

  switch (status) {
    case ngrt4n::OperationSucceeded:
      cssClass = "alert alert-success";
      logLevel = "info";
      break;
    case ngrt4n::OperationFailed:
      cssClass = "alert alert-warning";
      logLevel = "warn";
      break;
    case ngrt4n::OperationInProgress:
      cssClass = "alert alert-primary";
      logLevel = "error";
      break;
    default:
      m_infoBox.hide();
      hidden = true;
      break;
  }

  CORE_LOG(logLevel, msg);

  m_infoBox.setText(msg);
  m_infoBox.setStyleClass(cssClass);
  m_infoBox.setHidden(hidden);
}


Wt::WDialog* WebMainUI::createAboutDialog(void)
{
  Wt::WDialog* dialog = new Wt::WDialog(&m_mainWidget);
  dialog->setTitleBarEnabled(false);
  dialog->setStyleClass("Wt-dialog");
  
  Wt::WPushButton* closeButton(new Wt::WPushButton(tr("Close").toStdString()));
  closeButton->clicked().connect(dialog, &Wt::WDialog::accept);
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("about-tpl"), dialog->contents());

  tpl->bindString("version", PKG_VERSION.toStdString());
  tpl->bindString("corelib-version", ngrt4n::libVersion().toStdString());
  tpl->bindString("codename", REL_NAME.toStdString());
  tpl->bindString("release-id", REL_INFO.toStdString());
  tpl->bindString("release-year", REL_YEAR.toStdString());
  tpl->bindString("bug-report-email", REPORT_BUG.toStdString());
  tpl->bindWidget("close-button", closeButton);

  return dialog;
}


void WebMainUI::initOperatorDashboard(void)
{
  bindExecutiveViewWidgets();

  auto userViews = m_dbSession->listViewListByAssignedUser(m_dbSession->loggedUser().username);

  if (userViews.empty()) {
    m_operatorHomeTpl.bindEmpty("bi-report-title");
    m_operatorHomeTpl.bindEmpty("bi-report-dashlet");
    showMessage(ngrt4n::OperationFailed, Q_TR("No view loaded"));
    return ;
  }

  // Generate view thumbnails
  int thumbIndex = 0;
  int thumbPerRow = m_dbSession->dashboardTilesPerRow();
  std::string failedViews = "";
  for (const auto& view : userViews) {
    auto loadViewOut = loadView(view.path);
    if (! loadViewOut.first) {
      CORE_LOG("error", tr("%1: %2").arg(QString(view.name.c_str()), loadViewOut.second).toStdString());
      failedViews.append(failedViews.empty()? "": ", ").append(view.name);
      continue;
    }

    QObject::connect(loadViewOut.first, SIGNAL(dashboardSelected(std::string)), this, SLOT(handleDashboardSelected(std::string)));

    const auto DashboardName = loadViewOut.first->rootNode().name.toStdString();
    Wt::WTemplate* thumbWidget = createThumbnailWidget(loadViewOut.first->thumbnailTitleBar(),
                                                       loadViewOut.first->thumbnailProblemDetailBar(),
                                                       loadViewOut.first->thumbnail());
    thumbWidget->clicked().connect(std::bind(&WebMainUI::handleDashboardSelected, this, DashboardName));
    m_thumbsLayout->addWidget(thumbWidget, thumbIndex / thumbPerRow, thumbIndex % thumbPerRow); // take the ownership of the widget
    m_thumbsWidgets.insert(DashboardName, thumbWidget);

    ++thumbIndex;
  }

  showConditionalUiWidgets(userViews);

  if (thumbIndex > 0) {
    startDashbaordUpdate();
  }

  if (thumbIndex != static_cast<int>(userViews.size())) {
    showMessage(ngrt4n::OperationFailed, QObject::tr("Loading failures (details in log): %1").arg(failedViews.c_str()).toStdString());
  }
}


Wt::WTemplate* WebMainUI::createThumbnailWidget(Wt::WLabel* titleWidget, Wt::WLabel* problemWidget, Wt::WImage* imageWidget)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("dashboard-thumbnail.tpl"));
  tpl->setStyleClass("btn btn-unknown");
  tpl->bindWidget("thumb-titlebar", titleWidget);
  tpl->bindWidget("thumb-problem-details", problemWidget);
  tpl->bindWidget("thumb-image", imageWidget);
  return tpl;
}

void WebMainUI::clearThumbnailTemplate(Wt::WTemplate* tpl)
{
  tpl->takeWidget("thumb-titlebar");
  tpl->takeWidget("thumb-problem-details");
  tpl->takeWidget("thumb-image");
}

void WebMainUI::showConditionalUiWidgets(const DbViewsT& views)
{
  if (m_dbSession->isCompleteUserDashboard()) {
    m_biDashlet.initialize(views);
    m_operatorHomeTpl.bindString("bi-report-title", Q_TR("Reports"));
    m_operatorHomeTpl.bindWidget("bi-report-dashlet", &m_biDashlet);
  } else {
    m_operatorHomeTpl.bindEmpty("bi-report-title");
    m_operatorHomeTpl.bindEmpty("bi-report-dashlet");
    if (m_dbSession->displayOnlyTiles()) {
      doJavaScript("$('#ngrt4n-side-pane').hide();");
      doJavaScript("$('#ngrt4n-content-pane').removeClass().addClass('col-sm-12');");
    } else {
      doJavaScript("$('#ngrt4n-side-pane').show();");
      doJavaScript("$('#ngrt4n-content-pane').removeClass().addClass('col-sm-8');");
      doJavaScript("$('#ngrt4n-side-pane').removeClass().addClass('col-sm-4');");
    }
  }
}


void WebMainUI::bindExecutiveViewWidgets(void)
{
  m_thumbsContainer.setLayout(m_thumbsLayout = new Wt::WGridLayout());
  m_eventFeedsContainer.setLayout(m_eventFeedLayout = new Wt::WVBoxLayout());
  m_operatorHomeTpl.setTemplateText(Wt::WString::tr("operator-home.tpl"));
  m_operatorHomeTpl.bindWidget("info-box", &m_infoBox);
  m_operatorHomeTpl.bindWidget("thumbnails", &m_thumbsContainer);
  m_operatorHomeTpl.bindWidget("event-feeds", &m_eventFeedsContainer);
  m_dashboardStackedContents.addWidget(&m_operatorHomeTpl);
}

void WebMainUI::setInternalPath(const std::string& path)
{
  wApp->setInternalPath(path);
}


void WebMainUI::configureDialogWidget(void)
{
  m_uploadForm.setStyleClass("Wt-dialog");
  m_uploadForm.titleBar()->setStyleClass("titlebar");
  m_previewSelectorDialog.setStyleClass("Wt-dialog");
  m_previewSelectorDialog.titleBar()->setStyleClass("titlebar");

  // bind dialog events
  m_previewSelectorDialog.dataTriggered().connect(this, &WebMainUI::handlePreviewFile);
}


bool WebMainUI::createDirectory(const std::string& path, bool cleanContent)
{
  bool ret = false;
  QDir dir(path.c_str());
  if (! dir.exists() && ! dir.mkdir(dir.absolutePath())) {
    return false;
    QString errMsg = tr("Unable to create the directory (%1)").arg(dir.absolutePath());
    CORE_LOG("error", errMsg.toStdString());
    showMessage(ngrt4n::OperationFailed, errMsg.toStdString());
  }  else {
    ret = true;
    if (cleanContent) dir.remove("*");
  }
  return ret;
}


void WebMainUI::startDashbaordUpdate(void)
{
  Wt::WTimer* tmpTimer(new Wt::WTimer());
  tmpTimer->setInterval(2000);
  tmpTimer->start();
  tmpTimer->timeout().connect(std::bind([=](){tmpTimer->stop();
    delete tmpTimer; handleRefresh();}));
}


void WebMainUI::handleAuthSystemChanged(int authSystem)
{
  switch (authSystem) {
    case WebBaseSettings::LDAP:
      m_menuLinks[MenuLdapUsers]->setDisabled(false);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', false);");
      break;
    default:
      m_dbSession->deleteAuthSystemUsers(WebBaseSettings::LDAP);
      wApp->doJavaScript("$('#menu-ldap-users').prop('disabled', true);");
      break;
  }
}

void WebMainUI::handleLdapUsersMenu(void)
{
  if (m_authSettingsForm.getAuthenticationMode() != WebBaseSettings::LDAP) {
    showMessage(ngrt4n::OperationFailed, Q_TR("Denied, please enable LDAP authentication first"));
  } else {
    m_adminStackedContents.setCurrentWidget(m_ldapUserManager);
    if (m_ldapUserManager->updateUserList() <= 0) {
      showMessage(ngrt4n::OperationFailed, m_ldapUserManager->lastError());
    }
    m_adminPanelTitle.setText(Q_TR("LDAP Users"));
  }
}


void WebMainUI::handleBuiltInUsersMenu(void)
{
  m_adminStackedContents.setCurrentWidget(m_dbUserManager->dbUserListWidget());
  m_dbUserManager->updateDbUsers();
  m_adminPanelTitle.setText(Q_TR("Users"));
}


void WebMainUI::handleNewUserMenu(void)
{
  m_dbUserManager->userForm()->reset();
  m_adminStackedContents.setCurrentWidget(m_dbUserManager->userForm());
  m_adminPanelTitle.setText(Q_TR("Create New User"));
}


void  WebMainUI::handleViewAclMenu(void)
{
  m_adminStackedContents.setCurrentWidget(m_viewAccessPermissionForm);
  m_viewAccessPermissionForm->resetModelData();
  m_adminPanelTitle.setText(Q_TR("Manage Operations Views per User"));
}


void WebMainUI::handleDeleteView(const std::string& viewName)
{
  auto loadedDashboardItem = m_dashboardMap.find(viewName.c_str());
  if (loadedDashboardItem != m_dashboardMap.end()) {
    WebDashboard* dashboard = *loadedDashboardItem;
    m_dashboardStackedContents.removeWidget(dashboard);
    delete (*loadedDashboardItem);
    m_dashboardMap.remove(viewName.c_str());
  }
}


void WebMainUI::handleUserEnableStatusChanged(int status, std::string data)
{
  switch (status) {
    case LdapUserManager::EnableAuthSuccess:
      showMessage(ngrt4n::OperationSucceeded,
                  Q_TR("LDAP authentication enabled for user ") + data);
      break;
    case LdapUserManager::DisableAuthSuccess:
      showMessage(ngrt4n::OperationSucceeded,
                  Q_TR("LDAP authentication disabled for user ") + data);
      break;
    case LdapUserManager::GenericError:
      showMessage(ngrt4n::OperationFailed, data);
      break;
    default:
      break;
  }
}


WebMsgDialog* WebMainUI::createNotificationManager(void)
{
  WebMsgDialog* notificationManager = new WebMsgDialog(m_dbSession, &m_mainWidget);
  notificationManager->operationCompleted().connect(this, &WebMainUI::showMessage);
  return notificationManager;
}


void WebMainUI::setDashboardAsFrontStackedWidget(WebDashboard* dashboard)
{
  if (dashboard) {
    swicthFrontStackedWidgetTo(dashboard);
    dashboard->doJavascriptAutoResize();
    m_selectViewBox->setCurrentIndex( m_selectViewBox->findText(dashboard->rootNode().name.toStdString()) );
    m_showOnlyProblemMsgsField->setHidden(false);
    m_currentDashboard = dashboard;
  }
}

void WebMainUI::handleDashboardSelected(std::string viewName)
{
  auto loadedDashboardItem = m_dashboardMap.find(viewName.c_str());
  if (loadedDashboardItem != m_dashboardMap.end()) {
    setDashboardAsFrontStackedWidget(*loadedDashboardItem);
  }
}


void WebMainUI::swicthFrontStackedWidgetTo(Wt::WWidget* widget)
{
  if (widget) {
    m_dashboardStackedContents.setCurrentWidget(widget);
  }
}



void WebMainUI::handleImportDescriptionFile(void)
{
  if (m_fileUploader->empty()) {
    showMessage(ngrt4n::OperationFailed, Q_TR("No file selected"));
    return;
  }

  bool success = createDirectory(m_configDir.toStdString(), false); // false means don't clean the directory

  if (! success) {
    showMessage(ngrt4n::OperationFailed, Q_TR("Cannot create configuration directory"));
    return ;
  }

  QString tmpFileName(m_fileUploader->spoolFileName().c_str());
  CORE_LOG("info", QObject::tr("Parse uploaded file: %1").arg(tmpFileName).toStdString());

  CoreDataT cdata;
  Parser parser{&cdata,
        Parser::ParsingModeEditor,
        &m_settings,
        m_dbSession};

  auto&& outParser = parser.parse(tmpFileName);
  if (outParser.first != ngrt4n::RcSuccess) {
    showMessage(ngrt4n::OperationFailed, outParser.second.toStdString());
    return ;
  }

  std::string filename = m_fileUploader->clientFileName().toUTF8();
  QString destPath = QString("%1/%2").arg(m_configDir, filename.c_str());
  QFile file(tmpFileName);
  file.copy(destPath);
  file.remove();
  saveViewInfoIntoDatabase(cdata, destPath);
}


void WebMainUI::saveViewInfoIntoDatabase(const CoreDataT& cdata, const QString& path)
{
  DboView view;
  view.name = cdata.bpnodes[ngrt4n::ROOT_ID].name.toStdString();
  view.service_count = cdata.bpnodes.size() + cdata.cnodes.size();
  view.path = path.toStdString();

  auto addViewOut = m_dbSession->addView(view);
  if (addViewOut.first != ngrt4n::RcSuccess){
    showMessage(ngrt4n::OperationFailed, addViewOut.second.toStdString());
  } else {
    QString viewName(view.name.c_str());
    QString viewPath(view.path.c_str());
    QString srvCountStr = QString::number(view.service_count);
    std::string msg = QObject::tr("Added: %1 (%2 Services) - Path: %3"
                                  ).arg(viewName, srvCountStr, viewPath).toStdString();
    showMessage(ngrt4n::OperationSucceeded, msg);
    CORE_LOG("info", msg);
  }
}


void WebMainUI::resetFileUploader(void)
{
  Wt::WFileUpload* old = m_fileUploader;
  m_fileUploader = new Wt::WFileUpload();
  m_fileUploader->setProgressBar(new Wt::WProgressBar()); // take the ownership of the progressbar
  m_fileUploader->uploaded().connect(this, &WebMainUI::handleImportDescriptionFile);
  m_fileUploader->fileTooLarge().connect(this, &WebMainUI::handleUploadFileTooLarge);
  m_uploadForm.contents()->insertBefore(m_fileUploader, &m_uploadSubmitButton);
  if (old) {
    m_uploadForm.contents()->removeWidget(old);
    delete old;
  }
}
