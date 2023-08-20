#pragma once
#include<qfileiconprovider.h>

enum class IconProviderScope
{
    ContentBrowserTreeView,
    ContentBrowserListView,
    Other
};

class CustomFileIconProvider :
    public QFileIconProvider
{
public:
    CustomFileIconProvider()
        : QFileIconProvider()
    {
    }

    IconProviderScope _scope = IconProviderScope::Other;

private:
    QIcon icon(const QFileInfo& info) const;
};

