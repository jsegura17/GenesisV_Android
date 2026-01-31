package com.example.genesisv

import android.content.Intent
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.ListView
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity

/**
 * Submenú "Scenes OpenGL": 5 opciones de escenas 2D Platform. Cada una abre UnderConstructionActivity.
 * Botón atrás vuelve al menú principal.
 */
class ScenesMenuActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_scenes_menu)

        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                finish()
            }
        })

        val listView = findViewById<ListView>(R.id.list_scenes)
        val scenes = listOf(
            getString(R.string.scene_floor),
            getString(R.string.scene_background),
            getString(R.string.scene_static_obj),
            getString(R.string.scene_anim),
            getString(R.string.scene_player)
        )

        listView.adapter = ArrayAdapter(
            this,
            android.R.layout.simple_list_item_1,
            scenes
        )

        listView.setOnItemClickListener { _, _, position, _ ->
            if (position == 0) {
                startActivity(Intent(this, MainActivity::class.java).apply {
                    putExtra(MainActivity.EXTRA_SCENE_INDEX, 0)
                })
            } else {
                startActivity(Intent(this, UnderConstructionActivity::class.java).apply {
                    putExtra(UnderConstructionActivity.EXTRA_SCENE_INDEX, position)
                })
            }
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
