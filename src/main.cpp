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
            ccColor3B spamBackgroundColor3B = { 95, 46, 26 };
            ccColor3B spamTextColor3B = { 255, 51, 51 };

            // GJCommentListLayer->CustomListView->TableView->CCContentLayer-> (comments array)
            // m_cellArray = array of CellComment
            CCArray* cells = blv->m_tableView->m_cellArray;
            if (cells) {
                CCObject* obj;
                const std::string removedComment = "[BotCommentRemoval] Comment removed.";

                CCARRAY_FOREACH(cells, obj) {
                    if (auto commentCell = dynamic_cast<CommentCell*>(obj)) {
                        gd::string originalCommentString = commentCell->m_comment->m_commentString;
                        if (checkStringContainsBlacklistedKeyword(originalCommentString))
                        {
                            log::info("POSSIBLE SPAM ALERT:\n   Username: \"{}\"\n   Comment: \"{}\"", commentCell->m_comment->m_userName, originalCommentString);

                            CCObject* commentCellChild;
                            CCARRAY_FOREACH((CCArray*)(commentCell->getChildren()), commentCellChild) {
                                if (auto commentCCLayerColor = dynamic_cast<CCLayerColor*>(commentCellChild))
                                {
                                    commentCCLayerColor->setColor(spamBackgroundColor3B);
                                };

                                if (auto singularCommentLayer = dynamic_cast<CCLayer*>(commentCellChild)) {

                                    if (auto bitmapFontLabel = dynamic_cast<CCLabelBMFont*>(singularCommentLayer->getChildByID("comment-text-label"))) {
                                        bitmapFontLabel->setColor(spamTextColor3B);
                                        
                                        bitmapFontLabel->setCString(removedComment.c_str());

                                        if (auto mainCCMenu = dynamic_cast<CCMenu*>(singularCommentLayer->getChildByID("main-menu"))) {
                                            auto spr = ButtonSprite::create("Show Original");
                                            auto showOriginalStringButton = LambdaMenuItem::create(
                                                spr, [originalCommentString, bitmapFontLabel, removedComment](CCObject* sender) {
                                                    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
                                                    auto spr = static_cast<ButtonSprite*>(btn->getNormalImage());

                                                    std::string removedCommentText(removedComment);  // removedCommentText is output, convert const char* to gd::string 

                                                    // toggles the button's text between 'Show Original' and 'Hide Original'
                                                    if (bitmapFontLabel->getString() == removedCommentText) {
                                                        spr->setString("Hide Original");
                                                        bitmapFontLabel->setCString(originalCommentString.c_str()); // restore original comment
                                                    }
                                                    else {
                                                        spr->setString("Show Original");
                                                        bitmapFontLabel->setCString(removedCommentText.c_str()); // hide comment again
                                                    }

                                                    btn->setPosition(-36, -136.6f);
                                                    btn->setScale(0.33f);

                                                    CCPoint* point = new CCPoint();
                                                    point->x = 0;
                                                    point->y = 0;

                                                    spr->setPosition(*point);
                                                }
                                            );

                                            showOriginalStringButton->setPosition(-36, -136.6f);
                                            showOriginalStringButton->setScale(0.33f);
                                            mainCCMenu->addChild(showOriginalStringButton);
                                        }
                                    }
                                }
                            }

                        }
                    }
                }
            }
        }

        return true;
    }

    class LambdaMenuItem : public CCMenuItemSpriteExtra {
    public:
        std::function<void(CCObject*)> callback;

        static LambdaMenuItem* create(CCNode* normalSprite, std::function<void(CCObject*)> cb) {
            auto item = new LambdaMenuItem();
            if (item && item->initWithNormalSprite(normalSprite, nullptr, nullptr, nullptr, nullptr)) {
                item->autorelease();
                item->callback = std::move(cb);
                return item;
            }
            CC_SAFE_DELETE(item);
            return nullptr;
        }

        void activate() override {
            CCMenuItemSpriteExtra::activate();
            if (callback) callback(this);
        }
    };

    bool checkStringContainsBlacklistedKeyword(gd::string inputCommentString) {
        // older approach was std::transform but github said the build fails on android, so here's another solution
        for (char& c : inputCommentString) {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        std::vector<std::string> susWords = {
            "discord gg", ".gg/", ".gg", "discord.gg", "pU763A4kp7", "dsc.gg", "boost your", "boost youtube", "boost view", "boost like",
            "trade", "giftcard", "approved by robtop", "you want to get free", "to get free", "join my server", "check out my server", "join us",
            "free reward", "win a prize", "limited time", "get free", "free creator points", "official geometry", "official gd disc", "official gd serv", "check my level", "check my new level", "check out my new l", "check out my l", "my level"
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
