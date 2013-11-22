#include "resizer_unity.h"

ResizerUnity::ResizerUnity()
{
    QString app_name = "resizer.desktop";
    unity = unity_launcher_entry_get_for_desktop_id( app_name.toStdString().c_str() );
}

QString ResizerUnity::version()
{
    return QString(UNITY_PLUGIN_VERSION);
}

void ResizerUnity::updateProgressBar(int min, int max, int val)
{
    if(min>max)
        std::swap(min,max);
    if(val<0 || min<0 || max<=0){
        unity_launcher_entry_set_progress_visible(unity, false);
    }else{
        double progress = (val-min)/max;
        unity_launcher_entry_set_progress(unity, progress);
        unity_launcher_entry_set_progress_visible(unity, true);
    }
}

void ResizerUnity::updateNumber(int number)
{
    if(number<0){
        unity_launcher_entry_set_count_visible(unity, false);
    }else{
        unity_launcher_entry_set_count(unity, number);
        unity_launcher_entry_set_count_visible(unity, true);
    }
}

void ResizerUnity::finished()
{
    unity_launcher_entry_set_urgent (unity, true);
}

Q_EXPORT_PLUGIN2(ResizerUnity, ResizerUnity)
