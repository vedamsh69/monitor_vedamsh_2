// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Monitor.
//
// eProsima Fast DDS Monitor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Monitor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Monitor. If not, see <https://www.gnu.org/licenses/>.

import QtQuick 2.15
import QtQuick.Controls 2.15
import Theme 1.0

/*
Menu containing the possible actions that can be performed on topic entities.
*/
Menu {
    id: topicMenu
    property string domainEntityId: ""
    property string domainId: ""
    property string entityId: ""
    property string currentAlias: ""
    property string entityKind: ""
    height: 160
    width: 230

    // ========== NEW: Track subscription state ==========
    property bool isSubscribed: false
    
    // Update subscription state when menu is about to show
    onAboutToShow: {
        console.log("[TopicMenu] Menu opening for topic:", currentAlias);
        console.log("[TopicMenu] Domain label:", domainId);
        console.log("[TopicMenu] Domain entity id:", domainEntityId);

        // IMPORTANT:
        // domainId is a display label/alias in some views (often 1-based),
        // while domainEntityId maps to the real monitor domain key used by
        // Controller dynamic pub/sub APIs.
        var domain = parseInt(domainEntityId);
        console.log("[TopicMenu] Using controller domain id:", domain);
        
        // Check if this topic is currently subscribed
        isSubscribed = controller.isTopicSubscribed(domain, currentAlias);
        
        console.log("[TopicMenu] Subscription status:", isSubscribed ? "SUBSCRIBED" : "NOT SUBSCRIBED");
    }
    // ========== END NEW CODE ==========

    MenuItem {
        text: "Change alias"
        onTriggered: changeAlias(menu.domainEntityId, menu.entityId, menu.currentAlias, menu.entityKind)
    }
    
    MenuItem {
        text: "View problems"
        onTriggered: filterEntityStatusLog(menu.entityId)
    }
    
    MenuItem {
        text: "Filter graph view"
        onTriggered: openTopicView(menu.domainEntityId, menu.domainId, menu.entityId)
    }
    
    // ========== MODIFIED: Dynamic Subscribe/Unsubscribe button ==========
    MenuItem {
        // Dynamic text based on subscription state
        text: topicMenu.isSubscribed ? "Unsubscribe" : "Subscribe"
        
        onTriggered: {
            var domain = parseInt(topicMenu.domainEntityId);
            
            console.log("[TopicMenu] Button clicked!");
            console.log("[TopicMenu] Current subscription status:", topicMenu.isSubscribed);
            console.log("[TopicMenu] Domain ID:", domain);
            console.log("[TopicMenu] Topic name:", topicMenu.currentAlias);
            
            if (topicMenu.isSubscribed) {
                // ===== UNSUBSCRIBE ACTION =====
                console.log("[TopicMenu] ACTION: Unsubscribing from topic");
                controller.unsubscribeFromTopic(domain, topicMenu.currentAlias);
                console.log("[TopicMenu] Unsubscribe command sent to controller");
            } else {
                // ===== SUBSCRIBE ACTION =====
                console.log("[TopicMenu] ACTION: Subscribing to topic");
                console.log("[TopicMenu] Opening subscription dialog...");
                
                createsubscriptiondialogid.domainnumber = domain;
                createsubscriptiondialogid.topicname = topicMenu.currentAlias;
                createsubscriptiondialogid.open();
                
                console.log("[TopicMenu] Subscription dialog opened");
            }
        }
    }
    // ========== END MODIFIED CODE ==========
    
    MenuItem {
        text: "Publish"
        onTriggered: {
            publishDialogid.topicname = menu.currentAlias;
            publishDialogid.domainnumber = parseInt(menu.domainEntityId);
            publishDialogid.open();
        }
    }
}
