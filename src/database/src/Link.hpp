#ifndef DATABASE_AREA_LINK_HPP
#define DATABASE_AREA_LINK_HPP

namespace area {
    struct Link
    {
        int page_no = -1;
        int item_on_page_offset = -1;
        inline bool IsActive() const {
            return page_no > -1 && item_on_page_offset > -1;
        }
    };
}

#endif // DATABASE_AREA_LINK_HPP