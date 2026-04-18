package com.graphictablet

import android.net.Uri
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import androidx.appcompat.app.AppCompatActivity
import androidx.media3.common.MediaItem
import androidx.media3.common.PlaybackException
import androidx.media3.common.Player
import androidx.media3.exoplayer.ExoPlayer
import androidx.media3.datasource.DefaultDataSource
import androidx.media3.exoplayer.source.DefaultMediaSourceFactory
import com.graphictablet.databinding.ActivityMainBinding
import kotlinx.coroutines.*

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var player: ExoPlayer? = null
    private val scope = CoroutineScope(Dispatchers.Main + SupervisorJob())

    private val UDP_PORT = 9000
    private val DISCOVERY_PORT = 9001
    private val STREAM_URI = "udp://@:$UDP_PORT"

    private lateinit var discoveryService: DiscoveryService
    private var isStreaming = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupFullscreen()
        setupUI()
        startDiscovery()
        initPlayer()
    }

    // ─── Fullscreen ───────────────────────────────────────────────────────────

    private fun setupFullscreen() {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
            window.insetsController?.let {
                it.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                it.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_FULLSCREEN
                    or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            )
        }
    }

    // ─── UI ───────────────────────────────────────────────────────────────────

    private fun setupUI() {
        binding.surfaceView.setOnClickListener { toggleInfoOverlay() }
        binding.btnRetry.setOnClickListener {
            binding.overlayStatus.visibility = View.GONE
            initPlayer()
            startDiscovery()
        }
        val localIp = getLocalIpAddress()
        binding.tvInfo.text = "Este dispositivo: $localIp  •  Buscando server..."
    }

    private fun toggleInfoOverlay() {
        if (binding.overlayInfo.visibility == View.VISIBLE) {
            binding.overlayInfo.visibility = View.GONE
        } else {
            binding.overlayInfo.visibility = View.VISIBLE
            scope.launch {
                delay(3000)
                binding.overlayInfo.visibility = View.GONE
            }
        }
    }

    // ─── Discovery ────────────────────────────────────────────────────────────

    private fun startDiscovery() {
        val localIp = getLocalIpAddress()
        discoveryService = DiscoveryService(localIp, UDP_PORT, DISCOVERY_PORT)

        binding.tvStatus.text = "Buscando GraphicTablet Server en la red..."
        binding.progressBar.visibility = View.VISIBLE
        binding.overlayStatus.visibility = View.VISIBLE

        discoveryService.start(scope) { serverIp ->
            binding.tvInfo.text = "Este dispositivo: $localIp  •  Server: $serverIp"
            binding.tvStatus.text = "✓ Server encontrado ($serverIp)"
        }
    }

    // ─── ExoPlayer ────────────────────────────────────────────────────────────

    private fun initPlayer() {
        val dataSourceFactory = DefaultDataSource.Factory(this)
        player = ExoPlayer.Builder(this)
            .setMediaSourceFactory(DefaultMediaSourceFactory(this).setDataSourceFactory(dataSourceFactory))
            .build()
            .also { exoPlayer ->
                exoPlayer.setVideoSurfaceView(binding.surfaceView)
                exoPlayer.addListener(object : Player.Listener {
                    override fun onPlaybackStateChanged(state: Int) {
                        when (state) {
                            Player.STATE_BUFFERING -> {
                                binding.progressBar.visibility = View.VISIBLE
                            }
                            Player.STATE_READY -> {
                                isStreaming = true
                                binding.progressBar.visibility = View.GONE
                                binding.overlayStatus.visibility = View.GONE
                                binding.overlayInfo.visibility = View.GONE
                                discoveryService.stop()
                            }
                            else -> {
                                isStreaming = false
                                binding.progressBar.visibility = View.GONE
                            }
                        }
                    }
                    override fun onPlayerError(error: PlaybackException) {
                        isStreaming = false
                        binding.progressBar.visibility = View.GONE
                        binding.overlayStatus.visibility = View.VISIBLE
                        binding.tvError.text = "Error: ${error.message}"
                        binding.tvError.visibility = View.VISIBLE
                    }
                })
                exoPlayer.setMediaItem(MediaItem.fromUri(Uri.parse(STREAM_URI)))
                exoPlayer.playWhenReady = true
                exoPlayer.prepare()
            }
    }

    // ─── Lifecycle ────────────────────────────────────────────────────────────

    override fun onResume() { super.onResume(); setupFullscreen(); player?.play() }
    override fun onPause() { super.onPause(); player?.pause() }
    override fun onDestroy() {
        super.onDestroy()
        scope.cancel()
        if (::discoveryService.isInitialized) discoveryService.stop()
        player?.release()
        player = null
    }

    // ─── Utils ────────────────────────────────────────────────────────────────

    private fun getLocalIpAddress(): String {
        try {
            val interfaces = java.net.NetworkInterface.getNetworkInterfaces()
            for (intf in interfaces) {
                for (addr in intf.inetAddresses) {
                    if (!addr.isLoopbackAddress && addr is java.net.Inet4Address)
                        return addr.hostAddress ?: "desconocida"
                }
            }
        } catch (e: Exception) { e.printStackTrace() }
        return "desconocida"
    }
}
