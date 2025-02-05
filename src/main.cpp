#include <Geode/Geode.hpp>
#include <Geode/modify/GJCommentListLayer.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

class $modify(MyCommentListLayer, GJCommentListLayer) {
    bool init(BoomListView * blv, const char* title, ccColor4B color, float width, float height, bool blueBorder) {
        if (!GJCommentListLayer::init(blv, title, color, width, height, blueBorder)) {
            return false;
        }
        
        // if (!blv) just crashed my game so I did a NULL check
        if (blv != NULL)
        {
            // GJCommentListLayer->CustomListView->TableView->CCContentLayer-> (comments array)
            // m_cellArray = array of CellComment
            CCArray* cells = blv->m_tableView->m_cellArray;
            if (cells) {
                CCObject* obj;
                CCARRAY_FOREACH(cells, obj) {
                    if (auto commentCell = dynamic_cast<CommentCell*>(obj)) {
                        gd::string commentString = commentCell->m_comment->m_commentString;
                        if (checkStringContainsBlacklistedKeyword(commentString))
                        {
                            log::info("POSSIBLE SPAM ALERT:\n   Username: \"{}\"\n   Comment: \"{}\"", commentCell->m_comment->m_userName, commentString);
                            commentCell->setScale(0);
                            commentCell->draw();
                        }
                    }
                }
            }
        }

        return true;
    }

    bool checkStringContainsBlacklistedKeyword(gd::string inputCommentString) {
        // make string lowercase
        std::transform(inputCommentString.begin(), inputCommentString.end(), inputCommentString.begin(),
            [](unsigned char c) { return std::tolower(c); });

        std::vector<std::string> susWords = {
            ".gg/", ".gg", "discord.gg", "pU763A4kp7", "dsc.gg", "boost your", "boost youtube", "boost view", "boost like",
            "trade", "giftcard", "approved by robtop", "you want to get free", "to get free",  "join my server", "check out my server", "join us",
            "free reward", "win a prize", "limited time", "get free", "free creator points", "official geometry", "official gd disc", "official gd serv"
        };

        // check if the message contains ANY (even one!) banned word/phrase
        for (const auto& keyword : susWords) {
            if (inputCommentString.find(keyword) != std::string::npos) {
                return true; // Found a blacklisted keyword
            }
        }

        // no blacklisted words/phrases found
        return false; 
    }

};
