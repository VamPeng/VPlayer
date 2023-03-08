package com.vam.vplayer

import android.content.Context
import java.io.File
import java.io.FileOutputStream
import java.util.concurrent.Executors

object FileUtil {

    private const val File_Name = "waipo.mp4"

    fun getVideo(context: Context, func: (File) -> Unit) {

        var file = File(context.cacheDir, "video")
        if (!file.exists()) {
            file.mkdir()
        }

        val gifFile = File(file, File_Name)
        if (gifFile.exists()) {
            func.invoke(gifFile);return
        }

        val ins = context.assets.open(File_Name)
        val bytes = ByteArray(1024)

        gifFile.createNewFile()
        val ops = FileOutputStream(gifFile)
        var length = 0
        Executors.newSingleThreadExecutor().execute {
            ins.use {
                ops.use {
                    while (ins.read(bytes).also { length = it } != -1) {
                        ops.write(bytes, 0, length)
                    }
                }
                func.invoke(gifFile)
            }
        }
    }

}