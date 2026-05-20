#include <tesla.hpp>

namespace tsl {
    namespace elm {

        u32 ListItemDefaultHeight = 40;

        /**
         * @brief A item that goes into a list
         *
         */
        class MiniListItem : public ListItem {
        public:
            /**
             * @brief Constructor
             *
             * @param text Initial description text
             */
            MiniListItem(const std::string& text, const std::string& value = "")
                : ListItem(text, value) {
            }
            virtual ~MiniListItem() {}

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), ListItemDefaultHeight);
            }
        };
    }
}