package com.vam.vplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import android.widget.TextView
import com.vam.vplayer.databinding.ActivityMainBinding
import com.vam.vplayer.player.VPlayer

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private val player by lazy { VPlayer() }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        PermissionUtil.request(this)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        initSeekBar()

        player.setSurface(binding.mainSurface)

        player.setOnErrorListener { }
        player.setOnPrepareListener { player.start() }
        player.setOnProgressListener { }

        FileUtil.getVideo(this) {
            player.settDataSource(it.absolutePath)
        }

        binding.mainPlay.setOnClickListener {
            play()
        }

    }


    private fun initSeekBar() {
        binding.mainSeek.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }

        })
    }

    private fun play() {
        player.prepare()
    }

    companion object {
        init {
            System.loadLibrary("vplayer");
        }
    }


}