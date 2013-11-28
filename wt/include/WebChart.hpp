/*
 * WebChart.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 27-11-2013                                                 #
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

#ifndef WEBCHART_HPP
#define WEBCHART_HPP

#include <Wt/WStandardItemModel>
#include <Wt/Chart/WPieChart>
#include <Wt/WContainerWidget>
#include <Wt/WColor>
#include <Wt/WPaintDevice>

class WebChart : public Wt::WContainerWidget
{
public:
  WebChart();
  virtual ~WebChart();
  void setSeverityData(int _sev, int count);
  static Wt::WColor colorFromSeverity(const int& _sev);

private:
  Wt::Chart::WPieChart* m_chart;
  Wt::WStandardItemModel* m_model;
};

#endif // WEBCHART_HPP