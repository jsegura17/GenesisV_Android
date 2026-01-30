package com.example.genesisv

import android.os.Bundle
import android.view.View
import androidx.activity.OnBackPressedCallback
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {

    companion object {
        init {
            System.loadLibrary("genesisv")
        }
    }

    external fun setExampleIndex(index: Int)
    external fun setBackButtonLabelBitmap(width: Int, height: Int, pixels: ByteArray)

    /** Llamado desde native cuando el usuario toca "Back Menu"; cierra la actividad en el UI thread. */
    fun scheduleFinish() {
        runOnUiThread { finish() }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val index = intent?.getIntExtra(MenuActivity.EXTRA_EXAMPLE_INDEX, 0) ?: 0
        setExampleIndex(index)
        createBackButtonLabelBitmap()

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                finish()
            }
        })
    }

    private fun createBackButtonLabelBitmap() {
        val width = 256
        val height = 64
        val bitmap = android.graphics.Bitmap.createBitmap(width, height, android.graphics.Bitmap.Config.ARGB_8888)
        val canvas = android.graphics.Canvas(bitmap)
        canvas.drawColor(android.graphics.Color.TRANSPARENT)
        val paint = android.graphics.Paint().apply {
            color = android.graphics.Color.WHITE
            textSize = 32f
            isAntiAlias = true
        }
        val text = "Back Menu"
        val bounds = android.graphics.Rect()
        paint.getTextBounds(text, 0, text.length, bounds)
        canvas.drawText(text, (width - bounds.width()) / 2f, (height + bounds.height()) / 2f - 4, paint)
        val pixels = IntArray(width * height)
        bitmap.getPixels(pixels, 0, width, 0, 0, width, height)
        val bytes = ByteArray(pixels.size * 4)
        for (i in pixels.indices) {
            bytes[i * 4] = (pixels[i] shr 16 and 0xFF).toByte()
            bytes[i * 4 + 1] = (pixels[i] shr 8 and 0xFF).toByte()
            bytes[i * 4 + 2] = (pixels[i] and 0xFF).toByte()
            bytes[i * 4 + 3] = (pixels[i] shr 24 and 0xFF).toByte()
        }
        setBackButtonLabelBitmap(width, height, bytes)
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUi()
        }
    }

    private fun hideSystemUi() {
        val decorView = window.decorView
        decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }
}