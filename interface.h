// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Main gtkmm application interface.

#ifndef INTERFACE_H
#define INTERFACE_H

#include <exception>
#include <iostream>
#include <gtkmm.h>
#include <map>
#include <string>
#include <set>

using std::set;

#include "resources.h"
#include "keyvals.h"

template<typename ResType>
class Buddy_t : public Gtk::HBox {
	public:

		Buddy_t() : Gtk::HBox() , res(NULL) , pbuddylist(NULL) {}
		Buddy_t(const Buddy_t<ResType> & rhs) : \
		 Gtk::HBox() , label() , ebox() , name() , res(rhs.res) , \
		 pbuddylist(rhs.pbuddylist) {

			this->add(ebox);
			this->ebox.add(label);
			this->ebox.signal_button_press_event().connect(\
			 sigc::mem_fun(*this, &Buddy_t<ResType>::on_buddy_clicked) );
			this->label.set_alignment(Gtk::ALIGN_LEFT);
			this->show_all_children();
		}

		void set(const std::string & tempname, const std::string & temp_display) {

			name = tempname;
			nickname = temp_display;
			this->set_unfocused();

			sqlite3_result qres;
			res->get_db()("SELECT `reply` FROM `buddies` WHERE `name`='" + tempname + "';", qres);

			string reply = "";
			if (qres.size() == 0 || qres.front()["reply"].empty()) {
				reply = "\n<b>Reply Configured:</b> No";
			} else {
				//std::cout << "Reply is: " << qres.front()[""]
				reply = "\n<b>Reply Configured:</b> Yes";
			}

			label.set_tooltip_markup("<b>Name:</b> " + name + "\n"
			 "<b>Nickname:</b> " + nickname + reply);
		}

		void set_unfocused() {
			label.set_text(nickname);
		}

		bool on_buddy_clicked(GdkEventButton * event) {

			for (typename std::map<std::string, Buddy_t<ResType> >::iterator it = pbuddylist->begin(); \
			      it != pbuddylist->end(); ++it) {
				it->second.set_unfocused();
			}

			label.set_markup("<span "
			               " foreground=\"#333333\" "
			               " font-weight=\"ultrabold\" "
			             ">" + nickname + " &lt; </span>");

			res->get_current_buddy() = name;

			return true;
		}

		Buddy_t & SetResourcePointer(ResType * tempres) {
			//std::cout << "RESOURCE POINTER WAS SET!" << std::endl;
			res = tempres;
			return *this;
		}

		template<typename T>
		Buddy_t & SetListPointer(T ptr) {
			pbuddylist = ptr;
			return *this;
		}

		Gtk::Label label;
		Gtk::EventBox ebox;
		ResType * res;
		std::string name;
		std::string nickname;
		std::map<std::string, Buddy_t<ResType> > * pbuddylist;

	private:

		Buddy_t<ResType> & operator = (const Buddy_t<ResType> & rhs);

}; typedef Buddy_t<resources> Buddy;

class BuddyList {
	public:
		BuddyList(Gtk::Window & tempmainwin, resources & tempres, Gtk::VBox & tempvbox) : \
		 buddy_map() , mainwin(tempmainwin) , res(tempres) , buddy_vbox(tempvbox) {}

		Buddy & operator [] (const std::string & rhs) {
			return buddy_map[rhs].SetResourcePointer(&res).SetListPointer(&buddy_map);
		}

		std::map<std::string, Buddy>::iterator find(const std::string & rhs) {
			return buddy_map.find(rhs);
		}

		std::map<std::string, Buddy>::iterator end() { return buddy_map.end(); }

		std::map<std::string, Buddy>::iterator begin() { return buddy_map.begin(); }

		const bool erase(const std::string & name) {
			if (buddy_map.find(name) != buddy_map.end()) {
				buddy_map.erase(buddy_map.find(name));
				return true;
			}
			return false;
		}

		void set( const std::string & name     , \
		          const std::string & nickname , \
		          const std::string & reply    ) {

			try {

				(*this)[name].set(name, nickname);
				buddy_vbox.pack_start((*this)[name], Gtk::PACK_SHRINK);
				mainwin.show_all_children();

				res.get_db()("INSERT INTO `buddies` VALUES "
				                     "(  '" + name     + \
				                     "', '" + nickname + \
				                     "', '" + reply + "'  );"       );

			} catch (const char * ex) {
				std::cerr << "exception: " << ex << std::endl;
				// FIXME: popup window if exception
			}

		}

	private:
		std::map<std::string, Buddy> buddy_map;
		Gtk::Window & mainwin;
		resources & res;
		Gtk::VBox & buddy_vbox;
};

template<typename ResType> class InfoWindow_t : public Gtk::Window {
	public:

		InfoWindow_t(ResType & tempres) : table(13, 2, true) , res(tempres) {

			this->set_position(Gtk::WIN_POS_CENTER);
			this->set_title("Info");
			this->set_border_width(10);
			table.set_spacings(5);

table.attach(lversion      , 0, 1, 0 , 1 );       lversion.set_text("  version:  ");
table.attach(lbalance      , 0, 1, 1 , 2 );       lbalance.set_text("  balance:  ");
table.attach(lblocks       , 0, 1, 2 , 3 );        lblocks.set_text("  blocks:  ");
table.attach(lconnections  , 0, 1, 3 , 4 );   lconnections.set_text("  connections:  ");
table.attach(lproxy        , 0, 1, 4 , 5 );         lproxy.set_text("  proxy:  ");
table.attach(lgenerate     , 0, 1, 5 , 6 );      lgenerate.set_text("  generate:  ");
table.attach(lgenproclimit , 0, 1, 6 , 7 );  lgenproclimit.set_text("  genproclimit:  ");
table.attach(ldifficulty   , 0, 1, 7 , 8 );    ldifficulty.set_text("  difficulty:  ");
table.attach(lhashespersec , 0, 1, 8 , 9 );  lhashespersec.set_text("  hashespersec:  ");
table.attach(ltestnet      , 0, 1, 9 , 10);       ltestnet.set_text("  testnet:  ");
table.attach(lkeypoololdest, 0, 1, 10, 11); lkeypoololdest.set_text("  keypoololdest:  ");
table.attach(lpaytxfee     , 0, 1, 11, 12);      lpaytxfee.set_text("  paytxfee:  ");
table.attach(lerrors       , 0, 1, 12, 13);        lerrors.set_text("  errors:  ");

      lversion.set_alignment(Gtk::ALIGN_LEFT);
      lbalance.set_alignment(Gtk::ALIGN_LEFT);
       lblocks.set_alignment(Gtk::ALIGN_LEFT);
  lconnections.set_alignment(Gtk::ALIGN_LEFT);
        lproxy.set_alignment(Gtk::ALIGN_LEFT);
     lgenerate.set_alignment(Gtk::ALIGN_LEFT);
 lgenproclimit.set_alignment(Gtk::ALIGN_LEFT);
   ldifficulty.set_alignment(Gtk::ALIGN_LEFT);
 lhashespersec.set_alignment(Gtk::ALIGN_LEFT);
      ltestnet.set_alignment(Gtk::ALIGN_LEFT);
lkeypoololdest.set_alignment(Gtk::ALIGN_LEFT);
     lpaytxfee.set_alignment(Gtk::ALIGN_LEFT);
       lerrors.set_alignment(Gtk::ALIGN_LEFT);

      version.set_alignment(Gtk::ALIGN_LEFT);
      balance.set_alignment(Gtk::ALIGN_LEFT);
       blocks.set_alignment(Gtk::ALIGN_LEFT);
  connections.set_alignment(Gtk::ALIGN_LEFT);
        proxy.set_alignment(Gtk::ALIGN_LEFT);
     generate.set_alignment(Gtk::ALIGN_LEFT);
 genproclimit.set_alignment(Gtk::ALIGN_LEFT);
   difficulty.set_alignment(Gtk::ALIGN_LEFT);
 hashespersec.set_alignment(Gtk::ALIGN_LEFT);
      testnet.set_alignment(Gtk::ALIGN_LEFT);
keypoololdest.set_alignment(Gtk::ALIGN_LEFT);
     paytxfee.set_alignment(Gtk::ALIGN_LEFT);
       errors.set_alignment(Gtk::ALIGN_LEFT);

table.attach(version      , 1, 2, 0 , 1 );       version.set_text("<null>");
table.attach(balance      , 1, 2, 1 , 2 );       balance.set_text("<null>");
table.attach(blocks       , 1, 2, 2 , 3 );        blocks.set_text("<null>");
table.attach(connections  , 1, 2, 3 , 4 );   connections.set_text("<null>");
table.attach(proxy        , 1, 2, 4 , 5 );         proxy.set_text("<null>");
table.attach(generate     , 1, 2, 5 , 6 );      generate.set_text("<null>");
table.attach(genproclimit , 1, 2, 6 , 7 );  genproclimit.set_text("<null>");
table.attach(difficulty   , 1, 2, 7 , 8 );    difficulty.set_text("<null>");
table.attach(hashespersec , 1, 2, 8 , 9 );  hashespersec.set_text("<null>");
table.attach(testnet      , 1, 2, 9 , 10);       testnet.set_text("<null>");
table.attach(keypoololdest, 1, 2, 10, 11); keypoololdest.set_text("<null>");
table.attach(paytxfee     , 1, 2, 11, 12);      paytxfee.set_text("<null>");
table.attach(errors       , 1, 2, 12, 13);        errors.set_text("<null>");

			fr.set_label("Info");

			this->add(fr);
			fr.add(table);

			this->show_all_children();

		}

		void show_hooks() {

			std::map<string, string> mres;
			res.get_json_interface().getinfo(mres);

			      version.set_text("  " + mres["version"      ] + "  ");
			      balance.set_text("  " + mres["balance"      ] + "  ");
			       blocks.set_text("  " + mres["blocks"       ] + "  ");
			  connections.set_text("  " + mres["connections"  ] + "  ");
			        proxy.set_text("  " + mres["proxy"        ] + "  ");
			     generate.set_text("  " + mres["generate"     ] + "  ");
			 genproclimit.set_text("  " + mres["genproclimit" ] + "  ");
			   difficulty.set_text("  " + mres["difficulty"   ] + "  ");
			 hashespersec.set_text("  " + mres["hashespersec" ] + "  ");
			      testnet.set_text("  " + mres["testnet"      ] + "  ");
			keypoololdest.set_text("  " + mres["keypoololdest"] + "  ");
			     paytxfee.set_text("  " + mres["paytxfee"     ] + "  ");
			       errors.set_text("  " + mres["errors"       ] + "  ");

		}

	private:

		Gtk::Frame fr;

		Gtk::Table table;

		Gtk::Label lversion;
		Gtk::Label lbalance;
		Gtk::Label lblocks;
		Gtk::Label lconnections;
		Gtk::Label lproxy;
		Gtk::Label lgenerate;
		Gtk::Label lgenproclimit;
		Gtk::Label ldifficulty;
		Gtk::Label lhashespersec;
		Gtk::Label ltestnet;
		Gtk::Label lkeypoololdest;
		Gtk::Label lpaytxfee;
		Gtk::Label lerrors;

		Gtk::Label version;
		Gtk::Label balance;
		Gtk::Label blocks;
		Gtk::Label connections;
		Gtk::Label proxy;
		Gtk::Label generate;
		Gtk::Label genproclimit;
		Gtk::Label difficulty;
		Gtk::Label hashespersec;
		Gtk::Label testnet;
		Gtk::Label keypoololdest;
		Gtk::Label paytxfee;
		Gtk::Label errors;

		ResType & res;

}; typedef InfoWindow_t<resources> InfoWindow;

template<typename ResType> class SendWindow_t : public Gtk::Window {

	public:
		SendWindow_t(ResType & tempres) : \
		   vbox()           , \
		   balance_label()  , \
		   amount_label()   , \
		   amount_entry()   , \
		   address_label()  , \
		   address_entry()  , \
		   spacing()        , \
		   send("Send")     , \
			res(tempres)       {

			this->set_position(Gtk::WIN_POS_CENTER);
			this->set_title("Send Namecoins");
			this->set_default_size(400, 200);
			this->set_border_width(5);

			send.signal_clicked().connect( sigc::mem_fun(*this,
			        &SendWindow_t<ResType>::on_save_clicked) );

			this->add(vbox);

			vbox.pack_start(balance_label, Gtk::PACK_SHRINK);

			vbox.add(amount_label );
			vbox.add(amount_entry );
			vbox.add(address_label);
			vbox.add(address_entry);
			vbox.add(spacing);
			vbox.pack_start(send, Gtk::PACK_SHRINK);

			amount_label.set_text("Amount");
			address_label.set_text("Address");

			this->show_all_children();

		}

		void on_save_clicked() {

			string temp = amount_entry.get_text();
			string amount;

			for (size_t i = 0; i < temp.length(); i++) {
				if ((temp[i] >= '0' && temp[i] <= '9') || temp[i] == '.') {
					amount += temp[i];
				}
			}

			string address = address_entry.get_text();
			if (address.empty()) {

				Gtk::MessageDialog msg(*this, "You forgot to enter an address.");
				msg.run();
				return;
			}

			if (res.get_json_interface().validateaddress(address) == false) {
				Gtk::MessageDialog msg(*this, "Error: Invalid address.");
				msg.run();
				return;
			}

			if (temp == "0" || temp.empty()) {

				Gtk::MessageDialog msg(*this, "Invalid amount.");
				msg.run();
				return;
			}

			Gtk::MessageDialog dialog(*this, "Are you sure you want to send " + amount + " NMC?",
			    false, Gtk::MESSAGE_QUESTION,
			    Gtk::BUTTONS_OK_CANCEL);

			int result = dialog.run();

			if (result == Gtk::RESPONSE_OK) {

				int sendres = true;

				try {
					res.get_json_interface().send_to_address(address, amount);
				} catch (const std::exception & ex) {
					std::cout << "Send exception: " << ex.what() << std::endl;
					sendres = false;
				} catch (const char * ex) {
					sendres = false;
					std::cout << "const char *: " << ex << std::endl;
				} catch (...) {
					sendres = false;
				}

				if (sendres) {
					Gtk::MessageDialog msg(*this, "Send successful!");
					msg.run();
				} else {
					Gtk::MessageDialog msg(*this, "Send failed!");
					msg.run();
				}

			} else {

				Gtk::MessageDialog msg(*this, "Send not confirmed!");
				msg.run();

			}

			this->hide();

		}

		void show_hooks(const std::string & current_balance) {
			amount_entry.set_text("");
			address_entry.set_text("");
			balance_label.set_markup(current_balance);
		}

	private:

		Gtk::VBox   vbox;
		Gtk::Label  balance_label;
		Gtk::Label  amount_label;
		Gtk::Entry  amount_entry;
		Gtk::Label  address_label;
		Gtk::Entry  address_entry;
		Gtk::Label  spacing;
		Gtk::Button send;

		ResType & res;

}; typedef SendWindow_t<resources> SendWindow;

template<typename ResType> class TransactionsWindow_t : public Gtk::Window {

	public:
		TransactionsWindow_t(ResType & tempres) : CopyButton("Copy Txid to Clipboard") , DeleteButton("Delete Transaction") , CloseButton("Close") , res(tempres) {

			this->set_position(Gtk::WIN_POS_CENTER);
			this->set_title("Transactions");
			this->set_default_size(550, 350);

			add(m_VBox);
			m_ScrolledWindow.add(m_TreeView);
			m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

			m_VBox.pack_start(m_ScrolledWindow);

			m_Columns.add(m_col_time         );
			m_Columns.add(m_col_account      );
			m_Columns.add(m_col_category     );
			m_Columns.add(m_col_amount       );
			m_Columns.add(m_col_fee          );
			m_Columns.add(m_col_confirmations);
			m_Columns.add(m_col_address      );
			m_Columns.add(m_col_txid         );

			m_refTreeModel = Gtk::ListStore::create(m_Columns);
			m_TreeView.set_model(m_refTreeModel);

			m_TreeView.append_column("Time"          , m_col_time         );
			m_TreeView.append_column("Account"       , m_col_account      );
			m_TreeView.append_column("Category"      , m_col_category     );
			m_TreeView.append_column("Amount"        , m_col_amount       );
			m_TreeView.append_column("Fee"           , m_col_fee          );
			m_TreeView.append_column("Confirmations" , m_col_confirmations);
			m_TreeView.append_column("Address"       , m_col_address      );
			m_TreeView.append_column("Txid"          , m_col_txid         );

			CopyButton.signal_clicked().connect( sigc::mem_fun(*this,
			        &TransactionsWindow_t<ResType>::on_button_copy) );

			DeleteButton.signal_clicked().connect( sigc::mem_fun(*this,
			        &TransactionsWindow_t<ResType>::on_button_delete) );

			CloseButton.signal_clicked().connect( sigc::mem_fun(*this,
			        &TransactionsWindow_t<ResType>::on_button_close) );

			m_ButtonBox.set_border_width(5);
			m_ButtonBox.set_layout(Gtk::BUTTONBOX_END);

			m_VBox.pack_start(m_ButtonBox, Gtk::PACK_SHRINK);
			m_ButtonBox.pack_start(CopyButton, Gtk::PACK_SHRINK);
			m_ButtonBox.pack_start(DeleteButton, Gtk::PACK_SHRINK);
			m_ButtonBox.pack_start(CloseButton, Gtk::PACK_SHRINK);

			show_all_children();

		}

		void on_button_copy() {



			Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
			   m_TreeView.get_selection();

			Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

			if (iter) {
				Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
				refClipboard->set_text(Glib::ustring((*iter)[m_col_txid]));
			} else {

				Gtk::MessageDialog dialog(*this, "You forgot to select a transaction.");
				dialog.run();

			}


		}

		void on_button_delete() {


			Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
			   m_TreeView.get_selection();

			Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

			if (iter) {
				Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
				refClipboard->set_text(Glib::ustring((*iter)[m_col_txid]));

				Gtk::MessageDialog dialog(*this, "Are you sure you with to delete this transaction?",
				 false, Gtk::MESSAGE_QUESTION,
				 Gtk::BUTTONS_OK_CANCEL);

				int result = dialog.run();

				if (result == Gtk::RESPONSE_OK) {

					std::cout << "DELETING TRANSACTION: " << (*iter)[m_col_txid] << std::endl;

					bool success = true;
					try {
						std::stringstream ss;
						ss << (*iter)[m_col_txid];
						res.get_json_interface().deletetransaction(ss.str());
					} catch (...) {
						success = false;
					}

					if (success == false) {
						Gtk::MessageDialog msg(*this, "Failed to delete transaction.");
						msg.run();
					}

				} else {

					Gtk::MessageDialog delete_canceled(*this, "Operation cancelled.");

				}

			} else {

				Gtk::MessageDialog dialog(*this, "You forgot to select a transaction.");
				dialog.run();

			}


		}

		void on_button_close() {
			this->hide();
		}

		void delete_all_rows() {

			Gtk::TreeModel::Children::iterator it = m_refTreeModel->children().begin();
			Gtk::TreeModel::Children::iterator end = m_refTreeModel->children().end();

			if (it != end) {
				m_refTreeModel->erase(it);
				this->delete_all_rows();
			}

		}

		void add_row(std::map<string, string> & temprow) {

			Gtk::TreeModel::Row row = *(m_refTreeModel->append());
			row[m_col_account      ] = temprow["account"      ];
			row[m_col_category     ] = temprow["category"     ];
			row[m_col_amount       ] = temprow["amount"       ];
			row[m_col_fee          ] = temprow["fee"          ];
			row[m_col_confirmations] = temprow["confirmations"];
			row[m_col_address      ] = temprow["address"      ];
			row[m_col_txid         ] = temprow["txid"         ];

			time_t thetime(atol(temprow["time"].c_str()));
			struct tm * timeinfo = gmtime(&thetime);
			string restime = string(asctime(timeinfo));

			while (restime.find("\n") != std::string::npos)
				restime.erase(restime.find("\n"));

			row[m_col_time] = restime + " UTC";

		}

		void show_hooks() {

			this->delete_all_rows();

			vector<std::map<string, string> > rc;
			res.get_json_interface().listtransactions(rc);

			for (vector<std::map<string, string> >::iterator it = rc.begin(); \
			       it != rc.end(); ++it) {
				this->add_row(*it);
			}

		}

	private:

		Gtk::TreeModel::ColumnRecord m_Columns;
		Gtk::TreeModelColumn<string> m_col_account;
		Gtk::TreeModelColumn<string> m_col_category;
		Gtk::TreeModelColumn<string> m_col_amount;
		Gtk::TreeModelColumn<string> m_col_fee;
		Gtk::TreeModelColumn<string> m_col_confirmations;
		Gtk::TreeModelColumn<string> m_col_address;
		Gtk::TreeModelColumn<Glib::ustring> m_col_txid;
		Gtk::TreeModelColumn<string> m_col_time;

		Gtk::VBox m_VBox;

		Gtk::ScrolledWindow m_ScrolledWindow;
		Gtk::TreeView m_TreeView;
		Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

		Gtk::HButtonBox m_ButtonBox;
		Gtk::Button CopyButton;
		Gtk::Button DeleteButton;
		Gtk::Button CloseButton;

		ResType & res;

}; typedef TransactionsWindow_t<resources> TransactionsWindow;

template<typename ResType> class AddressesWindow_t : public Gtk::Window {

	public:
		AddressesWindow_t() : Copy_Button("Copy to Clipboard") , Close_Button("Close") {

			this->set_position(Gtk::WIN_POS_CENTER);

			this->set_title("Addresses");
			this->set_border_width(5);
			this->set_default_size(600, 400);


			add(m_VBox);
			m_ScrolledWindow.add(m_TreeView);
			m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

			m_VBox.pack_start(m_ScrolledWindow);
			m_VBox.pack_start(m_ButtonBox, Gtk::PACK_SHRINK);

			m_ButtonBox.pack_start(Copy_Button , Gtk::PACK_SHRINK);
			m_ButtonBox.pack_start(Close_Button, Gtk::PACK_SHRINK);

			m_ButtonBox.set_border_width(5);
			m_ButtonBox.set_layout(Gtk::BUTTONBOX_END);

			Copy_Button.signal_clicked().connect( sigc::mem_fun(*this,
			        &AddressesWindow_t<ResType>::on_button_copy) );

			Close_Button.signal_clicked().connect( sigc::mem_fun(*this,
			        &AddressesWindow_t<ResType>::on_button_close) );

			m_Columns.add(m_col_address);

			m_refTreeModel = Gtk::ListStore::create(m_Columns);
			m_TreeView.set_model(m_refTreeModel);

			m_TreeView.append_column("Address"  , m_col_address);

			show_all_children();

		}

		void delete_all_rows() {
			Gtk::TreeModel::Children::iterator it = m_refTreeModel->children().begin();
			Gtk::TreeModel::Children::iterator end = m_refTreeModel->children().end();

			if (it != end) {
				m_refTreeModel->erase(it);
				this->delete_all_rows();
			}

		}

		void add_address(const std::string & address) {
			Gtk::TreeModel::Row row = *(m_refTreeModel->append());
			row[m_col_address  ] = address;
		}

		void on_button_copy() {

			Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
				m_TreeView.get_selection();

			Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

			if (iter) {
				Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
				refClipboard->set_text((*iter)[m_col_address]);
				std::cout << "WAS SELECTED: " << (*iter)[m_col_address] << std::endl;
			} else {

				Gtk::MessageDialog dialog(*this, "You forgot to select an address to copy.");
				dialog.run();

			}

		}

		void on_button_close() {
			this->hide();
		}

	private:
		Gtk::TreeModel::ColumnRecord m_Columns;
		Gtk::TreeModelColumn<Glib::ustring> m_col_address;

		Gtk::VBox m_VBox;

		Gtk::ScrolledWindow m_ScrolledWindow;
		Gtk::TreeView m_TreeView;
		Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

		Gtk::HButtonBox m_ButtonBox;
		Gtk::Button Copy_Button;
		Gtk::Button Close_Button;

}; typedef AddressesWindow_t<resources> AddressesWindow;

template<typename ResType>
class EditNicknameWindow_t : public Gtk::Window {

	public:

		EditNicknameWindow_t(ResType & tempres, BuddyList & tempbuds) : \
		    save("Save") , res(tempres) , buddies(tempbuds) , separator() {

			this->set_position(Gtk::WIN_POS_CENTER);

			save.signal_clicked().connect(
			 sigc::mem_fun(*this, &EditNicknameWindow_t<ResType>::on_save_clicked));

			this->signal_delete_event().connect( \
			 sigc::mem_fun(*this, &EditNicknameWindow_t<ResType>::on_window_closed));

			this->set_title("Edit Nickname");
			fr.set_label("Edit Nickname");
			fr.set_border_width(10);
			this->add(fr);
			vbox.set_border_width(10);
			fr.add(vbox);

			nicknamelabel.set_text("Nickname");
			vbox.add(nicknamelabel);
			nicknameentry.set_width_chars(20);
			vbox.add(nicknameentry);

			vbox.add(separator);


			vbox.add(save);

			this->show_all_children();

		}

		bool on_window_closed(GdkEventAny *) {
			std::cout << "Window was closed" << std::endl;
			nicknameentry.set_text("");
			this->hide();
			return true;
		}

		void show_hooks(const std::string & nickname) {
			nicknameentry.set_text(nickname);
		}

		void on_save_clicked() {
			std::cout << "Save clicked." << std::endl;

			if (res.get_current_buddy().empty()) {
				this->hide();
				Gtk::MessageDialog msg(*this, "No buddy currently selected.");
				msg.run();
				return;
			}

			if (nicknameentry.get_text().empty()) {
				Gtk::MessageDialog msg(*this, "You forgot to enter a new nickname!");
				msg.run();
				return;
			}

			res.get_db()("UPDATE `buddies` SET `alias`='" + nicknameentry.get_text() + "' WHERE name='" + res.get_current_buddy() + "';");

			buddies[res.get_current_buddy()].set(res.get_current_buddy(), nicknameentry.get_text());
			this->on_window_closed(NULL);
		}

	private:

		Gtk::Frame fr;
		Gtk::VBox vbox;

		Gtk::Label nicknamelabel;
		Gtk::Entry nicknameentry;

		Gtk::HSeparator separator;

		Gtk::Button save;

		ResType & res;

		BuddyList & buddies;


}; typedef EditNicknameWindow_t<resources> EditNicknameWindow;



template<typename ResType> class PreferenceWindow_t : public Gtk::Window {

	public:

		PreferenceWindow_t(ResType & tempres) : save("Save") , res(tempres) {

			this->set_position(Gtk::WIN_POS_CENTER);

			save.signal_clicked().connect(
			 sigc::mem_fun(*this, &PreferenceWindow_t<ResType>::on_save_clicked));

			this->set_title("Preferences");

			fr.set_label("Preferences");
			fr.set_border_width(10);

			this->add(fr);

			vbox.set_border_width(10);

			fr.add(vbox);

			namecoinUsernameLabel.set_text("\nNamecoin Username");
			vbox.add(namecoinUsernameLabel);
			namecoinUsernameEntry.set_width_chars(20);
			vbox.add(namecoinUsernameEntry);

			namecoinPasswordLabel.set_text("\nNamecoin Password");
			vbox.add(namecoinPasswordLabel);
			namecoinPasswordEntry.set_width_chars(20);
			namecoinPasswordEntry.set_visibility(false);
			vbox.add(namecoinPasswordEntry);

			namecoinHostLabel.set_text("\nNamecoin Host");
			vbox.add(namecoinHostLabel);
			namecoinHostEntry.set_width_chars(20);
			vbox.add(namecoinHostEntry);

			namecoinPortLabel.set_text("\nNamecoin Port");
			vbox.add(namecoinPortLabel);
			namecoinPortEntry.set_width_chars(6);
			vbox.add(namecoinPortEntry);

			vbox.pack_start(separator);

			vbox.add(save);

			this->show_all_children();

		}

		void set_text_fields() {
			namecoinUsernameEntry.set_text(Get(res.get_db(), "json_user"));
			namecoinPasswordEntry.set_text(Get(res.get_db(), "json_pass"));
			namecoinHostEntry.set_text(Get(res.get_db(), "json_host"));
			namecoinPortEntry.set_text(Get(res.get_db(), "json_port"));
		}

		void on_save_clicked() {

			string newuser = namecoinUsernameEntry.get_text();
			string newpass = namecoinPasswordEntry.get_text();
			string newhost = namecoinHostEntry.get_text();
			string newport = namecoinPortEntry.get_text();

			if (newhost.empty()) {
				Gtk::MessageDialog msg(*this, "Error: Invalid host.");
				msg.run();
				return;
			}

			if (atoi(newport.c_str()) < 1 || atoi(newport.c_str()) > 65535) {
				Gtk::MessageDialog msg(*this, "Error: Invalid port.");
				msg.run();
				return;
			}

			Set(res.get_db(), "json_user", newuser);
			Set(res.get_db(), "json_pass", newpass);
			Set(res.get_db(), "json_host", newhost);
			Set(res.get_db(), "json_port", newport);

			res.get_json_interface().SetUser(newuser);
			res.get_json_interface().SetPass(newpass);
			res.get_json_interface().SetHost(newhost);
			res.get_json_interface().SetPort(newport);

			std::cout << "Save clicked!!" << std::endl;
			this->hide();
		}

	private:

		Gtk::Frame fr;

		Gtk::VBox vbox;

		Gtk::Label namecoinUsernameLabel;
		Gtk::Entry namecoinUsernameEntry;

		Gtk::Label namecoinPasswordLabel;
		Gtk::Entry namecoinPasswordEntry;

		Gtk::Label namecoinHostLabel;
		Gtk::Entry namecoinHostEntry;

		Gtk::Label namecoinPortLabel;
		Gtk::Entry namecoinPortEntry;

		Gtk::HSeparator separator;

		Gtk::Button save;

		ResType & res;

}; typedef PreferenceWindow_t<resources> PreferenceWindow;



template<typename ResType> class TransferNameWindow_t : public Gtk::Window {
	public:
		TransferNameWindow_t(ResType & tempres) :
		 res(tempres) , button("Send") {

			this->set_position(Gtk::WIN_POS_CENTER);
			set_title("Transfer Name");
			set_border_width(5);

			fr.set_label("Transfer Name");
			this->add(fr);

			fr.add(vbox);

			label.set_text("Receiving Address");

			vbox.add(label);
			vbox.add(entry);

			entry.set_width_chars(40);

			vbox.pack_start(separator, Gtk::PACK_EXPAND_WIDGET);

			vbox.add(button);

			button.signal_clicked().connect( sigc::mem_fun(*this,
                 &TransferNameWindow_t<ResType>::on_button_clicked) );

			this->show_all_children();

		}

		void show_hooks(const string & tempname) {
			name = tempname;
		}

		void on_button_clicked() {

			if (res.get_json_interface().validateaddress(entry.get_text())) {

				std::cout << "ADDRESS WAS VALID!" << std::endl;

				res.get_json_interface().transfer_name(entry.get_text(), name);
				this->hide();

			} else {

				Gtk::MessageDialog msg(*this, "Address to send to is invalid.");
				msg.run();

			}

		}

	private:

		Gtk::Frame fr;
		Gtk::VBox vbox;
		Gtk::Label label;
		Gtk::Entry entry;

		Gtk::HSeparator separator;

		Gtk::Button button;
		ResType & res;

		string name;

}; typedef TransferNameWindow_t<resources> TransferNameWindow;



template<typename ResType> class ConfigNameWindow_t : public Gtk::Window {
	public:
		ConfigNameWindow_t(ResType & tempres) :
		 res(tempres) , configure("Configure") , radio_auto("Auto Configure") , radio_custom("Custom Value") , auto_config(true) {

			this->set_border_width(10);

			vbox.set_spacing(2);

			this->set_title("Configure Name");

			this->set_position(Gtk::WIN_POS_CENTER);

			fr.set_label("Configure Name");
			fr.set_border_width(10);
			this->add(fr);

			fr.add(vbox);

			group = radio_auto.get_group();
			radio_custom.set_group(group);

			radio_auto.set_active();

			spacing2.set_text(" ");
			vbox.pack_start(spacing2, Gtk::PACK_SHRINK);

			method.set_text("Method");

			vbox.pack_start(method, Gtk::PACK_SHRINK);

			vbox.pack_start(radio_auto, Gtk::PACK_SHRINK);
			vbox.pack_start(radio_custom, Gtk::PACK_SHRINK);

			spacing.set_text(" ");
			vbox.pack_start(spacing, Gtk::PACK_SHRINK);

			radio_auto.signal_clicked().connect(sigc::mem_fun(*this, &ConfigNameWindow_t<ResType>::on_group_changed));

			configure.signal_clicked().connect(sigc::mem_fun(*this, &ConfigNameWindow_t<ResType>::on_configure_clicked));

			textview.get_buffer()->set_text("");

			vbox.pack_start(textview, Gtk::PACK_SHRINK);
			textviewspacing.set_text(" ");
			vbox.pack_start(textviewspacing, Gtk::PACK_SHRINK);

			vbox.pack_start(configure, Gtk::PACK_SHRINK);

			this->show_all_children();
			textviewspacing.hide();
			textview.hide();

		}

		void show_hooks(const string & tempname) {
			string val = res.get_json_interface().name_get_value(tempname);
			textview.get_buffer()->set_text(val);
			name = tempname;
		}

		void on_configure_clicked() {

			string theval = "some_onion_value";

			if (auto_config == false) {
				theval = textview.get_buffer()->get_text();
			}

			try {
				res.get_json_interface().name_update(name, theval);
				Gtk::MessageDialog msg(*this, "Name updated. It will take 1 block to take effect.");
				msg.run();
			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error updating name!");
				msg.run();
			}

			this->hide();

		}

		void on_group_changed() {

			if (radio_auto.get_active()) {
				std::cout << "Clicked auto!" << std::endl;

				textviewspacing.hide();
				textview.hide();

				auto_config = true;

			} else {

				textviewspacing.show();
				textview.show();

				auto_config = false;

			}

		}

	private:

		Gtk::TextView textview;
		Gtk::Label textviewspacing;

		Gtk::Label spacing;
		Gtk::Label spacing2;

		string name;
		bool auto_config;

		Gtk::Frame fr;
		Gtk::VBox vbox;
		Gtk::Label method;
		Gtk::RadioButton radio_auto, radio_custom;
		Gtk::RadioButtonGroup group;
		Gtk::Button configure;

		ResType & res;

}; typedef ConfigNameWindow_t<resources> ConfigNameWindow;




template<typename ResType> class IdentityWindow_t : public Gtk::Window {

	public:
		IdentityWindow_t(ResType & tempres) :
		 m_Button_Use("Use Identity") ,
		 m_Button_Configure("Configure") ,
		 m_Button_Transfer("Transfer") ,
		 transfernamewindow(tempres) ,
		 configurenamewindow(tempres) ,
		 res(tempres) {

			this->set_position(Gtk::WIN_POS_CENTER);

			set_title("Identities");
			set_border_width(5);
			set_default_size(300, 500);

			add(m_VBox);
			m_ScrolledWindow.add(m_TreeView);
			m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

			m_VBox.pack_start(m_ScrolledWindow);
			m_VBox.pack_start(m_ButtonBox, Gtk::PACK_SHRINK);

			m_ButtonBox.pack_start(m_Button_Use      , Gtk::PACK_SHRINK);
			m_ButtonBox.pack_start(m_Button_Configure, Gtk::PACK_SHRINK);
			m_ButtonBox.pack_start(m_Button_Transfer , Gtk::PACK_SHRINK);

			m_ButtonBox.set_border_width(5);
			m_ButtonBox.set_layout(Gtk::BUTTONBOX_END);

			m_Button_Use.signal_clicked().connect( sigc::mem_fun(*this,
			        &IdentityWindow_t<ResType>::on_button_use) );

			m_Button_Configure.signal_clicked().connect( sigc::mem_fun(*this,
			        &IdentityWindow_t<ResType>::on_button_configure) );

			m_Button_Transfer.signal_clicked().connect( sigc::mem_fun(*this,
			        &IdentityWindow_t<ResType>::on_button_transfer) );

			m_Columns.add(m_col_name  );
			m_Columns.add(m_col_status);

			m_refTreeModel = Gtk::ListStore::create(m_Columns);
			m_TreeView.set_model(m_refTreeModel);

			m_TreeView.append_column("Name"  , m_col_name);
			m_TreeView.append_column("Status", m_col_status);

			show_all_children();
		}

		void on_button_transfer() {

			Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
			   m_TreeView.get_selection();

			Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

			if (iter) {
				string selected_name = (*iter)[m_col_name];
				res.get_current_identity() = selected_name;

				transfernamewindow.show_hooks((*iter)[m_col_name]);
				transfernamewindow.show();

			} else {
				Gtk::MessageDialog msg(*this, "You forgot to choose an identity.");
				msg.run();
			}

		}

		void add_row(const std::string & name, const std::string & status, bool selected = false) {

			Gtk::TreeModel::Row row = *(m_refTreeModel->append());
         row[m_col_name  ] = name;
         row[m_col_status] = status;


			if (selected) {
				Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
				 m_TreeView.get_selection();

				refTreeSelection->select(row);
			}

		}

		void delete_all_rows() {
			Gtk::TreeModel::Children::iterator it = m_refTreeModel->children().begin();
			Gtk::TreeModel::Children::iterator end = m_refTreeModel->children().end();

			if (it != end) {
				m_refTreeModel->erase(it);
				this->delete_all_rows();
			}

		}

		void on_button_use() {

			Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
			   m_TreeView.get_selection();

			Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

			if (iter) {
				string selected_name = (*iter)[m_col_name];
				res.get_current_identity() = selected_name;
				Gtk::MessageDialog msg(*this, "Current identity set: " + selected_name);
				msg.run();
			} else {
				Gtk::MessageDialog msg(*this, "You forgot to choose an identity.");
				msg.run();
			}
		}

		void on_button_configure() {

			Glib::RefPtr<Gtk::TreeSelection> refTreeSelection =
			   m_TreeView.get_selection();

			Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

			if (iter) {
				string selected_name = (*iter)[m_col_name];
				configurenamewindow.show_hooks(selected_name);
				configurenamewindow.show();
			} else {
				Gtk::MessageDialog msg(*this, "You forgot to choose an identity.");
				msg.run();
			}

		}

	private:
		Gtk::TreeModel::ColumnRecord m_Columns;
		Gtk::TreeModelColumn<string> m_col_name;
		Gtk::TreeModelColumn<string> m_col_status;

		Gtk::VBox m_VBox;

		Gtk::ScrolledWindow m_ScrolledWindow;
		Gtk::TreeView m_TreeView;
		Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

		Gtk::HButtonBox m_ButtonBox;
		Gtk::Button m_Button_Use;
		Gtk::Button m_Button_Configure;
		Gtk::Button m_Button_Transfer;

		TransferNameWindow  transfernamewindow;
		ConfigNameWindow configurenamewindow;

		ResType & res;

}; typedef IdentityWindow_t<resources> IdentityWindow;


template<typename ResType> class NewIdentityWindow_t : public Gtk::Window {

	public:
		NewIdentityWindow_t(ResType & tempres) : radio_auto("Auto Configure") , radio_custom("Custom Value") , textview() , save("Save") , res(tempres) {

			this->set_title("New Identity");

			this->set_position(Gtk::WIN_POS_CENTER);

			save.signal_clicked().connect(
			 sigc::mem_fun(*this, &NewIdentityWindow_t<ResType>::on_save_clicked));

			this->signal_delete_event().connect( \
			 sigc::mem_fun(*this, &NewIdentityWindow_t<ResType>::on_window_closed));

			fr.set_label("New Identity Info");
			fr.set_border_width(10);
			this->add(fr);

			vbox.set_border_width(10);
			fr.add(vbox);

			newidentitylabel.set_text("Name (e.g. d/myname)");

			vbox.pack_start(newidentitylabel, Gtk::PACK_SHRINK);
			newidentityname.set_width_chars(20);
			vbox.pack_start(newidentityname, Gtk::PACK_SHRINK);

			vbox.add(spacing);

			method.set_text("Method");
			vbox.add(method);

			group = radio_auto.get_group();
			radio_custom.set_group(group);

			radio_auto.set_active();

			vbox.add(radio_auto);
			vbox.add(radio_custom);

			radio_auto.signal_clicked().connect(sigc::mem_fun(*this, &NewIdentityWindow_t<ResType>::on_group_changed));

			vbox.add(spacing2);

			textviewspacing.set_text("\n");

			textview.get_buffer()->set_text("");

			vbox.add(textview);
			vbox.add(textviewspacing);

			vbox.pack_start(separator);

			vbox.pack_start(save, Gtk::PACK_SHRINK);

			this->show_all_children();

		}

		void show_hooks() {
			textviewspacing.hide();
			textview.hide();
			auto_config = true;
		}

		void on_group_changed() {

			if (radio_auto.get_active()) {
				std::cout << "Clicked auto!" << std::endl;

				textviewspacing.hide();
				textview.hide();

				auto_config = true;

			} else {

				textviewspacing.show();
				textview.show();

				auto_config = false;

				std::cout << "Clicked custom!" << std::endl;
			}
			std::cout << "Group changed!" << std::endl;
		}

		void on_save_clicked() {

			string theval = "onion_value";

			if (auto_config == false) {
				theval = textview.get_buffer()->get_text();
			}

			std::cout << "THE VAL: " << theval << std::endl;

			if (newidentityname.get_text() == "") {
				Gtk::MessageDialog msg(*this, "You forgot to enter a name!");
				msg.run();
				return;
			}

			string temp_reply;
			vector<string> temp_relays;
			int rc = 0;

			try {
				res.get_json_interface().name_scan(newidentityname.get_text(), temp_reply, temp_relays);
			} catch (const json_unexpected &) {

				std::cout << "RC is 1 because there was an exception!" << std::endl;

				rc = 1;
			}

			if (rc) {
				Gtk::MessageDialog msg(*this, "Name already exits on the network.");
				msg.run();
				return;
			}

			sqlite3_result qres;
			res.get_db()("SELECT * FROM `pending_identities` WHERE name='" + newidentityname.get_text() + "';", qres);

			if (qres.size()) {
				Gtk::MessageDialog msg(*this, "This identity is already pending confirmation on the network.");
				msg.run();
				return;
			}

			rc = res.get_json_interface().\
			 name_new_and_firstupdate(newidentityname.get_text(), theval);

			if (rc == 3) {

				Gtk::MessageDialog msg(*this, "name_new failed: didn't return enough hashes.");
				msg.run();

			} else if (rc == 2) {

				Gtk::MessageDialog msg(*this, "name_new failed: got an exception.");
				msg.run();

			} else if (rc == 1) {

				Gtk::MessageDialog msg(*this, "name_firstupdate failed: got an exception.");
				msg.run();

			} else {

				Gtk::MessageDialog msg(*this, "Success! Your new identity will show up as pending for a while.");
				res.get_db()("INSERT INTO `pending_identities` VALUES ('" + newidentityname.get_text() + "');");
				msg.run();

			}

			this->hide();
		}

		bool on_window_closed(GdkEventAny *) {

			textview.get_buffer()->set_text("");
			newidentityname.set_text("");
         this->hide();
         return true;
		}

	private:

		Gtk::VBox vbox;

		Gtk::Frame fr;

		Gtk::Label newidentitylabel;
		Gtk::Entry newidentityname;

		Gtk::RadioButton radio_auto, radio_custom;

		Gtk::RadioButtonGroup group;

		Gtk::Label textviewspacing;
		Gtk::TextView textview;

		Gtk::Label method;
		Gtk::Label spacing2;
		Gtk::Label spacing;
		Gtk::HSeparator separator;

		Gtk::Button save;

		ResType & res;

		bool auto_config;

}; typedef NewIdentityWindow_t<resources> NewIdentityWindow;

template<typename ResType> class NewBuddyWindow_t : public Gtk::Window {

	public:

		NewBuddyWindow_t(ResType & tempres, BuddyList & tempbuds) : \
		  save("Save") , res(tempres) , buddies(tempbuds) {

			this->set_title("New Buddy");

			this->set_position(Gtk::WIN_POS_CENTER);


			save.signal_clicked().connect(
			 sigc::mem_fun(*this, &NewBuddyWindow_t<ResType>::on_save_clicked));

			this->signal_delete_event().connect( \
			 sigc::mem_fun(*this, &NewBuddyWindow_t<ResType>::on_window_closed));

			fr.set_label("New Buddy Info");
			fr.set_border_width(10);
			this->add(fr);

			vbox.set_border_width(10);
			fr.add(vbox);

			newbuddylabel.set_text("Name");

			vbox.add(newbuddylabel);
			newbuddyname.set_width_chars(20);
			vbox.add(newbuddyname);


			newbuddynicknamelabel.set_text("\nNickname");
			vbox.add(newbuddynicknamelabel);
			newbuddynickname.set_width_chars(20);
			vbox.add(newbuddynickname);

			spacing.set_text("\n");
			vbox.add(spacing);

			vbox.add(save);

			this->show_all_children();
		}

		bool on_window_closed(GdkEventAny *) {

			newbuddyname.set_text("");
			newbuddynickname.set_text("");

			this->hide();

			return true;
		}

		void on_save_clicked() {

			if (buddies.find(newbuddyname.get_text()) != buddies.end()) { // FIXME: check if buddy exists
				Gtk::MessageDialog dialog(*this, "Buddy is already in your list.");
				dialog.run();
			} else {

				std::string reply;

				std::vector<std::string> relays;
				bool rc = false;
				try {
					rc = res.get_json_interface().name_scan(newbuddyname.get_text(), reply, relays);

				} catch (const json_unexpected & e) {
					rc = true;
					std::cout << "Buddy had malformed json. We'll add them anyway." << std::endl;
				}

				if (rc) {
					std::cout << "Buddy found on network!" << std::endl;
					for (std::vector<std::string>::iterator it = relays.begin(); \
					    it != relays.end(); ++it) {
						std::cout << "Relay: " << *it << std::endl;
					}

					string nick = newbuddynickname.get_text();
					if (nick.empty()) {
						nick = newbuddyname.get_text();
					}

					buddies.set(newbuddyname.get_text(), nick, reply);
				} else {
					std::cout << "Buddy not found on network!" << std::endl;
					Gtk::MessageDialog dialog(*this, "Buddy not found on network!");
					dialog.run();
				}
			}
			this->on_window_closed(NULL);
		}

	private:
		Gtk::Frame fr;
		Gtk::VBox vbox;
		Gtk::Label newbuddylabel;
		Gtk::Entry newbuddyname;

		Gtk::Label newbuddynicknamelabel;
		Gtk::Entry newbuddynickname;

		Gtk::Label spacing;

		Gtk::Button save;

		ResType & res;

		BuddyList & buddies;

}; typedef NewBuddyWindow_t<resources> NewBuddyWindow;

void gui_loop(resources & res, Gtk::Statusbar * statusbar) {

	for (; ; usleep(10000000)) {

		int numconnections = -1;

		try {
			numconnections = res.get_json_interface().getconnectioncount();
		} catch(...) {}

		std::stringstream ss;

		if (numconnections < 0) {
			ss << "Can't connect.";
		} else if (numconnections == 0) {
			ss << "No connections.";
		} else {
			ss << numconnections << " connection";
			if (numconnections > 1) ss << "s";
		}

		statusbar->pop();
		statusbar->push(ss.str());

	}

}

template<typename ResType> class MainWindow_t : public Gtk::Window {

	public:

		MainWindow_t(ResType & tempres)     : \
		   res(tempres)                     , \
		   Buddies(*this, res, buddy_vbox)  , \
		   vbox()                           , \
		   vbox2()                          , \
		   buddy_vbox()                     , \
		   scrolledWindow()                 , \
		   newbuddywindow(res, Buddies)     , \
		   editnicknamewindow(res, Buddies) , \
		   identitywindow(res)              , \
		   newidentitywindow(res)           , \
		   preferencewindow(res)            , \
			sendwindow(res)                  , \
		   transactionswindow(res)          , \
			infowindow(res)                  , \
		   statusbar()                        {

			boost::thread gui_loop_thread(boost::bind(&gui_loop, res, &(this->GetStatusbar())));

			this->set_position(Gtk::WIN_POS_CENTER);

			aboutdialog.set_program_name("voice.bit");
			aboutdialog.set_copyright("Copyright (C) 2011. voice.bit developers. All Rights Reserved.");
			aboutdialog.set_version("0.1 Alpha");

			this->set_title("voice.bit");
			this->set_default_size(250, 450);

			this->add(vbox);



			m_refActionGroup = Gtk::ActionGroup::create();

			m_refActionGroup->add(Gtk::Action::create("ActionsMenu", "_Actions"));
			m_refActionGroup->add(Gtk::Action::create("ActionsNewBuddy", "_New Buddy"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_new_buddy));
			m_refActionGroup->add(Gtk::Action::create("ActionsNewIdentity", "New Iden_tity"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_new_identity));
			m_refActionGroup->add(Gtk::Action::create("ActionsDeleteBuddy", "_Delete Buddy"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_delete_buddy));
			m_refActionGroup->add(Gtk::Action::create("ActionsSendNamecoins", "Send Na_mecoins"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_send_namecoins));
			m_refActionGroup->add(Gtk::Action::create("ActionsQuit", Gtk::Stock::QUIT),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_actions_quit));

			m_refActionGroup->add(Gtk::Action::create("ViewMenu", "_View"));
			m_refActionGroup->add(Gtk::Action::create("ViewAddresses", "Vie_w Addresses"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_view_addresses));
			m_refActionGroup->add(Gtk::Action::create("ViewIdentities", "View _Identities"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_view_identities));
			m_refActionGroup->add(Gtk::Action::create("ViewTransactions", "View _Transactions"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_view_transactions));
			m_refActionGroup->add(Gtk::Action::create("ViewInfo", "View Inf_o"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_view_info));

			m_refActionGroup->add(Gtk::Action::create("EditMenu", "_Edit"));
			m_refActionGroup->add(Gtk::Action::create("EditEditNickname", "Edit Nic_kname"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_edit_nickname));
			m_refActionGroup->add(Gtk::Action::create("EditPreferences", "_Preferences"),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_preferences));

			m_refActionGroup->add( Gtk::Action::create("HelpMenu", "_Help") );
			m_refActionGroup->add( Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT),
			 sigc::mem_fun(*this, &MainWindow_t::on_menu_about) );

			m_refUIManager = Gtk::UIManager::create();
			m_refUIManager->insert_action_group(m_refActionGroup);

			this->add_accel_group(m_refUIManager->get_accel_group());

			Glib::ustring ui_info =
			"<ui>"
			"  <menubar name='MenuBar'>"
			"    <menu action='ActionsMenu'>"
			"      <menuitem action='ActionsNewBuddy'/>"
			"      <menuitem action='ActionsDeleteBuddy'/>"
			"      <menuitem action='ActionsNewIdentity'/>"
			"      <menuitem action='ActionsSendNamecoins'/>"
			"      <separator/>"
			"      <menuitem action='ActionsQuit'/>"
			"    </menu>"
			"    <menu action='EditMenu'>"
			"      <menuitem action='EditPreferences'/>"
			"      <menuitem action='EditEditNickname'/>"
			"    </menu>"
			"    <menu action='ViewMenu'>"
			"      <menuitem action='ViewAddresses'/>"
			"      <menuitem action='ViewIdentities'/>"
			"      <menuitem action='ViewTransactions'/>"
		   "      <menuitem action='ViewInfo'/>"
			"    </menu>"
			"    <menu action='HelpMenu'>"
			"      <menuitem action='HelpAbout'/>"
			"    </menu>"
			"  </menubar>"
			"</ui>";

			m_refUIManager->add_ui_from_string(ui_info);
			Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");

			if (pMenubar)
				vbox.pack_start(*pMenubar, Gtk::PACK_SHRINK);

			scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
			scrolledWindow.add(buddy_vbox);

			vbox.pack_start(ebox, Gtk::PACK_EXPAND_WIDGET);

			ebox.add(scrolledWindow);

			sqlite3_result rc;
			res.get_db()("SELECT * FROM `buddies`;", rc);

			for (sqlite3_result::iterator it = rc.begin(); \
			           it != rc.end(); ++it) {
				if (Buddies.find((*it)["name"]) == Buddies.end())
					Buddies.set((*it)["name"], (*it)["alias"], (*it)["reply"]);
			}

			status.append_text("Inbound: Buddies");
			status.append_text("Inbound: Everyone");
			status.append_text("Inbound: Off");

			int tempmode = res.get_inbound_mode();

			if (tempmode == 0) {
				status.set_active_text("Inbound: Off");
			} else if (tempmode == 1) {
				status.set_active_text("Inbound: Buddies");
			} else if (tempmode == 2) {
				status.set_active_text("Inbound: Everyone");
			}

			status.signal_changed().connect(sigc::mem_fun(*this, &MainWindow_t<ResType>::on_status_changed));

			vbox.pack_start(vbox2, Gtk::PACK_SHRINK);

			vbox2.add(status);

			vbox.pack_start(statusbar, Gtk::PACK_SHRINK);

			this->show_all_children();

		}

		void on_status_changed() {
			std::cout << "Status changed!" << std::endl;

			       if (status.get_active_text() == "Inbound: Off"     ) {
				res.set_inbound_mode(0);
			} else if (status.get_active_text() == "Inbound: Buddies" ) {
				res.set_inbound_mode(1);
			} else if (status.get_active_text() == "Inbound: Everyone") {
				res.set_inbound_mode(2);
			}
		}

		void on_menu_view_info() {
			try {
				infowindow.show_hooks();
				infowindow.show();
			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error viewing info. Are you connected?");
				msg.run();
			}
		}

		void on_menu_new_identity() {

			try {

				double balance         = res.get_json_interface().get_balance();
				double firstupdate_fee = res.get_json_interface().calculate_fee();

				if (balance < firstupdate_fee) {

					Gtk::MessageDialog msg(*this, "You do not have enough credit to create a new identity.");
					msg.run();

				} else {

					if (balance < firstupdate_fee + 0.02) {
						Gtk::MessageDialog warn(*this, "You might not have enough credit if the netork is charging fees at this time.");
						warn.run();
					}

					newidentitywindow.show_hooks();
					newidentitywindow.show();

				}

			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error showing interface. Are you connected?");
				msg.run();
			}

		}

		void on_menu_about() {
			aboutdialog.run();
			aboutdialog.hide();
		}

		void on_menu_send_namecoins() {

			try {

				double bal = res.get_json_interface().get_balance();

				std::stringstream ss;
				ss << "Balance: " << bal;

				sendwindow.show_hooks(ss.str());
				sendwindow.show();

			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error looking up balance. Are you connected?");
				msg.run();
			}

		}

		void on_menu_view_addresses() {

			try {

				addresseswindow.delete_all_rows();

				std::vector<std::string> addresses;
				res.get_json_interface().getaddressesbyaccount("", addresses);

				for (std::vector<std::string>::iterator it = addresses.begin(); \
				         it != addresses.end(); ++it) {
					addresseswindow.add_address(*it);
				}

				addresseswindow.show();

			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error viewing addresses. Are you connected?");
				msg.run();
			}

		}

		void on_menu_view_transactions() {
			try {
				transactionswindow.show_hooks();
				transactionswindow.show();
			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error viewing transactions. Are you connected?");
				msg.run();
			}
		}

		void on_menu_view_identities() {

			try {

				identitywindow.delete_all_rows();

				vector<string> identities;
				res.get_json_interface().name_list(identities);

				sqlite3_result qres;
				res.get_db()("SELECT * FROM `pending_identities`;", qres);

				set<string> all_registered;

				for (vector<string>::iterator it = identities.begin(); \
				           it != identities.end(); ++it) {
					identitywindow.add_row(*it, "Registered", res.get_current_identity() == *it);
					all_registered.insert(*it);
				}

				for (sqlite3_result::iterator it = qres.begin(); it != qres.end(); ++it) {

					std::cout << "FOUND A PENDING IDENTITY: " << (*it)["name"] << std::endl;


					if (all_registered.find((*it)["name"]) == all_registered.end()) {
						identitywindow.add_row((*it)["name"], "Pending", res.get_current_identity() == (*it)["name"]);

					} else {
						res.get_db()("DELETE FROM `pending_identities` WHERE name='" + (*it)["name"] + "';");
					}
				}

				identitywindow.show();

			} catch (...) {
				Gtk::MessageDialog msg(*this, "Error showing identities: Are you connected?");
				msg.run();
			}

		}

		void on_menu_preferences() {
			preferencewindow.set_text_fields();
			preferencewindow.show();
		}

		void on_menu_edit_nickname() {

			if (res.get_current_buddy().empty()) {
				Gtk::MessageDialog msg(*this, "Error: No buddy selected.");
				msg.run();
				return;
			}

			editnicknamewindow.show_hooks(Buddies[res.get_current_buddy()].nickname);
			editnicknamewindow.show();
		}

		void on_menu_delete_buddy() {

			if (res.get_current_buddy().empty()) {
				Gtk::MessageDialog msg(*this, "Error: No buddy selected.");
				msg.run();
				return;
			}

			Gtk::MessageDialog dialog(*this, "Are you sure you with to delete this buddy?",
			    false, Gtk::MESSAGE_QUESTION,
			    Gtk::BUTTONS_OK_CANCEL);

			int result = dialog.run();

			if (result == Gtk::RESPONSE_OK) {
				std::cout << "Buddy delete was confirmed." << std::endl;

				int rc = Buddies.erase(res.get_current_buddy());

				if (rc == false) {
					Gtk::MessageDialog msg(*this, "Couldn't erase buddy: none selected.");
				} else {
					res.get_db()("DELETE FROM `buddies` WHERE name='" + res.get_current_buddy() + "';");
					res.get_current_buddy() = "";
				}


			} else {
				std::cout << "Buddy delete was not confirmed." << std::endl;
			}
		}

		void on_menu_new_buddy() {
			newbuddywindow.show();
		}

		void on_menu_actions_quit() { this->hide(); }

		void status_loop() {

			while (true) {

				try {
					std::cout << "checking status.." << std::endl;

				} catch (...) {

					std::cout << "Unhandled exception in status thread!" << std::endl;
				}
				usleep(1000000);
			}
		}

		Gtk::Statusbar & GetStatusbar() { return statusbar; }

	private:

		ResType & res;

		Gtk::VBox vbox;
		Gtk::VBox vbox2;
		Gtk::VBox buddy_vbox;
		Gtk::EventBox ebox;
		Gtk::ScrolledWindow scrolledWindow;

		BuddyList Buddies;

		Gtk::ComboBoxText status;

		Glib::RefPtr<Gtk::UIManager> m_refUIManager;
		Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

		Gtk::Statusbar statusbar;

		Gtk::AboutDialog aboutdialog;

		NewBuddyWindow      newbuddywindow;
		PreferenceWindow    preferencewindow;
		EditNicknameWindow  editnicknamewindow;
		IdentityWindow      identitywindow;
		AddressesWindow     addresseswindow;
		NewIdentityWindow   newidentitywindow;
		SendWindow          sendwindow;
		TransactionsWindow  transactionswindow;
		InfoWindow          infowindow;

};

#endif
