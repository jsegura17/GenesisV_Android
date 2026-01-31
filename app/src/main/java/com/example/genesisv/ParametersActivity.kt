package com.example.genesisv

import android.os.Bundle
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.SwitchCompat

/**
 * Pantalla de parámetros: opciones de la app (rotación de pantalla, etc.).
 * El estado se guarda en SharedPreferences.
 */
class ParametersActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_parameters)

        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                finish()
            }
        })

        val prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE)
        val switchRotation = findViewById<SwitchCompat>(R.id.switch_rotation)

        switchRotation.isChecked = prefs.getBoolean(KEY_SCREEN_ROTATION, true)
        switchRotation.setOnCheckedChangeListener { _, isChecked ->
            prefs.edit().putBoolean(KEY_SCREEN_ROTATION, isChecked).apply()
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }

    companion object {
        const val PREFS_NAME = "app_params"
        const val KEY_SCREEN_ROTATION = "screen_rotation_enabled"
    }
}
