package com.vam.vplayer;

import android.Manifest;
import android.app.Activity;
import android.content.Context;

public class PermissionUtil {

    public static void request(Activity activity){
        activity.requestPermissions(new String[]{
                Manifest.permission.WRITE_EXTERNAL_STORAGE
        },101);
    }

}
