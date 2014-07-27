/*
 * WebSettingUI.hpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 23-03-2014                                                 #
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

#ifndef WEBSESSION_HPP
#define WEBSESSION_HPP

#include "Preferences.hpp"
#include <Wt/WDialog>
#include <Wt/WRadioButton>
#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WSpinBox>
#include <Wt/WCheckBox>
#include <Wt/WObject>
#include <Wt/WStringListModel>
#include <Wt/WLengthValidator>
#include <memory>

class QString;


class WebPreferences : public Preferences, public Wt::WContainerWidget
{
public:
  enum InputTypeT {
    SourceTypeInput=1,
    SourceIndexInput=2
  };

  enum AuthenticationModeT {
    BuiltIn = 0,
    LDAP = 1
  };

  WebPreferences(void);
  virtual ~WebPreferences();
  virtual void setEnabledInputs(bool enable);
  Wt::Signal<std::string>& errorOccurred() { return m_errorOccurred; }
  void hideUnrequiredFields(void);
  void showAuthSettings(void);
  void showMonitoringSettings(void);
  QString getLdapServerUri(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_SERVER_URI);}
  QString getLdapBindUserDn(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_BIND_USER_DN);}
  QString getLdapSearchBase(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_SEARCH_BASE);}
  QString getLdapBindUserPassword(void) const { return m_settings->keyValue(Settings::AUTH_LDAP_BIND_USER_PASSWORD);}
  QString getLdapDnFormat(void) const{ return m_settings->keyValue(Settings::AUTH_LDAP_DN_FORMAT);}
  int getLdapVersion(void) const;
  int getAuthenticationMode(void) const;

protected:
  virtual void applyChanges(void);
  void handleCancel(void) {return;}
  virtual void fillFromSource(int _index);
  virtual void updateAllSourceWidgetStates(void);
  virtual void updateFields(void);
  virtual void saveAsSource(const qint32& index, const QString& type);
  virtual void addAsSource(void);
  virtual void deleteSource(void);

private:
  Wt::Signal<std::string> m_errorOccurred;

  // monitoring settings properties
  std::unique_ptr<Wt::WComboBox> m_sourceBox;
  std::unique_ptr<Wt::WStringListModel> m_sourceBoxModel;
  std::unique_ptr<Wt::WLineEdit> m_monitorUrlField;
  std::unique_ptr<Wt::WLineEdit> m_authStringField;
  std::unique_ptr<Wt::WLineEdit> m_livestatusHostField;
  std::unique_ptr<Wt::WLineEdit> m_livestatusPortField;
  std::unique_ptr<Wt::WComboBox> m_monitorTypeField;
  std::unique_ptr<Wt::WCheckBox> m_showAuthStringField;
  std::unique_ptr<Wt::WCheckBox> m_useNgrt4ndField;
  std::unique_ptr<Wt::WCheckBox> m_dontVerifyCertificateField;
  std::unique_ptr<Wt::WSpinBox> m_updateIntervalField;
  std::unique_ptr<Wt::WPushButton> m_applyChangeBtn;
  std::unique_ptr<Wt::WPushButton> m_addAsSourceBtn;
  std::unique_ptr<Wt::WPushButton> m_deleteSourceBtn;

  // auth settings properties
  std::unique_ptr<Wt::WComboBox> m_authenticationMode;
  std::unique_ptr<Wt::WLineEdit> m_ldapServerUri;
  std::unique_ptr<Wt::WLineEdit> m_ldapBindUserDn;
  std::unique_ptr<Wt::WLineEdit> m_ldapBindUserPassword;
  std::unique_ptr<Wt::WLineEdit> m_ldapDNFormat;
  std::unique_ptr<Wt::WLineEdit> m_ldapSearchBase;
  std::unique_ptr<Wt::WPushButton> m_saveAuthSettingsBtn;

  void promptUser(int inputType);
  void handleInput(const std::string& input, int inputType);
  int getSourceGlobalIndex(int sourceBoxIndex);
  int findSourceIndexInBox(int sourceGlobalIndex);
  void addToSourceBox(int sourceGlobalIndex);
  void bindFormWidget(void);
  void saveAuthSettings(void);
  void loadAuthSettings(void);
};

#endif // WEBSESSION_HPP
