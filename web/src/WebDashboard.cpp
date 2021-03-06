/*
 * WebDashboard.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2013                                                 #
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

#include "WebUtils.hpp"
#include "WebDashboard.hpp"
#include "Base.hpp"
#include "utilsCore.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <fstream>
#include <QDebug>
#include <Wt/WApplication>
#include <Wt/WPanel>
#include <Wt/WPointF>
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WImage>
#include <Wt/WTemplate>



WebDashboard::WebDashboard(DbSession* dbSession)
  : DashboardBase(dbSession)
{
  setCData(&m_cdata);
  bindFormWidgets();
  addJsEventScript();
  map()->containerSizeChanged().connect(this, & WebDashboard::hanleRenderingAreaSizeChanged);
}

WebDashboard::~WebDashboard()
{
  unbindWidgets();
}

std::pair<int, QString> WebDashboard::initialize(BaseSettings* p_settings, const QString& viewFile)
{
  auto outInitilization = DashboardBase::initialize(p_settings, viewFile);
  if (outInitilization.first != ngrt4n::RcSuccess) {
    CORE_LOG("error", outInitilization.second.toStdString());
  } else {
    m_thumbnailTitleBar.setText( rootNode().name.toStdString() );
    m_thumbnailProblemDetailsBar.setText("");
  }
  return outInitilization;
}

void WebDashboard::buildTree(void)
{
  m_tree.build();
  m_tree.expandNodeById(ngrt4n::ROOT_ID);
}


void WebDashboard::updateTree(const NodeT& _node, const QString& _tip)
{
  m_tree.updateItemDecoration(_node, _tip);
}

void WebDashboard::updateMsgConsole(const NodeT& node)
{
  if (node.sev != ngrt4n::Normal) {
    m_msgConsole.updateNodeMsg(node);
  } else {
    if (! m_showOnlyProblemMsgsState) {
      m_msgConsole.updateNodeMsg(node);
    }
  }
}

void WebDashboard::updateChart(void)
{
  CheckStatusCountT statsData;
  qint32 statCount = extractStatsData(statsData);
  m_chart.updateStatsData(statsData, statCount);
  m_chart.repaint();
}

void WebDashboard::buildMap(void)
{
  m_map.drawMap();
}


void WebDashboard::updateMap(const NodeT& _node, const QString& _tip)
{
  m_map.updateNode(_node, _tip);
}

void WebDashboard::updateThumbnailInfo(void)
{
  m_thumbnailProblemDetailsBar.setText(m_chart.problemsDetailsText());
}


void WebDashboard::updateMap(void)
{
  m_map.drawMap();
}

void WebDashboard::bindFormWidgets(void)
{
  setLayout(m_mainLayout = new Wt::WHBoxLayout());

  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  m_leftVBoxLayout.setContentsMargins(0, 0, 0, 0);
  m_rightVBoxLayout.setContentsMargins(0, 0, 0, 0);

  m_mainLayout->setSpacing(2);
  m_leftVBoxLayout.setSpacing(2);
  m_rightVBoxLayout.setSpacing(2);

  m_leftVBoxLayout.addWidget(&m_tree);
  m_leftVBoxLayout.addWidget(&m_chart);

  m_rightVBoxLayout.addWidget(m_map.renderingScrollArea());
  m_rightVBoxLayout.addWidget(&m_msgConsole);
  m_mainLayout->addLayout(&m_leftVBoxLayout);
  m_mainLayout->addLayout(&m_rightVBoxLayout);

  m_leftVBoxLayout.setResizable(0, true);
  m_mainLayout->setResizable(0, true);
  m_mainLayout->setResizable(1, true);
  m_leftVBoxLayout.setResizable(0, true);
  m_leftVBoxLayout.setResizable(1, true);
  m_rightVBoxLayout.setResizable(0, true);
  m_rightVBoxLayout.setResizable(1, true);
}


void WebDashboard::unbindWidgets(void)
{
  m_eventFeedLayout.clear();
  m_leftVBoxLayout.removeWidget(&m_tree);
  m_leftVBoxLayout.removeWidget(&m_chart);
  m_rightVBoxLayout.removeWidget(m_map.renderingScrollArea());
  m_rightVBoxLayout.removeWidget(&m_msgConsole);
  m_mainLayout->removeItem(&m_leftVBoxLayout);
  m_mainLayout->removeItem(&m_rightVBoxLayout);
  clear();
}


void WebDashboard::addJsEventScript(void)
{
  setJavaScriptMember("wtResize", JS_AUTO_RESIZING_FUNCTION);
  doJavascriptAutoResize();
}


void WebDashboard::updateEventFeeds(const NodeT &node)
{
  EventFeedItemsT::Iterator feed = m_eventFeedItems.find(node.id);
  if (feed != m_eventFeedItems.end()) {
    m_eventFeedLayout.removeWidget(*feed);
    delete *feed;
    m_eventFeedItems.erase(feed);
  }

  if (node.sev != ngrt4n::Normal) {
    Wt::WWidget* widget = createEventFeedTpl(node);
    m_eventFeedLayout.insertWidget(0, widget);
    m_eventFeedItems.insert(node.id, widget);
  }
}


Wt::WWidget* WebDashboard::createEventFeedTpl(const NodeT& node)
{
  Wt::WTemplate* tpl = new Wt::WTemplate(Wt::WString::tr("event-feed.tpl"));
  Wt::WAnchor* anchor = new Wt::WAnchor(Wt::WLink("#"), node.child_nodes.toStdString());

  std::string viewName = rootNode().name.toStdString();

  anchor->clicked().connect(std::bind(&WebDashboard::handleDashboardSelected, this, viewName));

  //FIXME: clear widget
  tpl->bindWidget("event-feed-title", anchor);
  tpl->bindString("severity-css-class", ngrt4n::severityCssClass(node.sev));
  tpl->bindString("event-feed-icon", ngrt4n::NodeIcons[node.icon]);
  tpl->bindString("event-feed-details", node.check.alarm_msg);
  tpl->bindString("platform", viewName);
  tpl->bindString("timestamp", ngrt4n::wTimeToNow(node.check.last_state_change));
  return tpl;
}


void WebDashboard::refreshMsgConsoleOnProblemStates(void)
{
  setDisabled(true);
  m_msgConsole.clearAll();
  for(const auto& node: m_cdata.cnodes) {
    updateMsgConsole(node);
  }
  setDisabled(false);
}
