package com.example.genesisv

import android.content.Intent
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.ListView
import androidx.appcompat.app.AppCompatActivity

/**
 * Menú principal: Ejemplos OpenGL (submenú con los 15 ejemplos) y Scenes OpenGL (submenú con 5 escenas).
 */
class MenuActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_menu)

        val listView = findViewById<ListView>(R.id.list_examples)
        val mainOptions = listOf(
            getString(R.string.menu_ejemplos_opengl),
            getString(R.string.menu_scenes_opengl),
            getString(R.string.menu_parametros),
            getString(R.string.menu_exit_app)
        )

        listView.adapter = ArrayAdapter(
            this,
            android.R.layout.simple_list_item_1,
            mainOptions
        )

        listView.setOnItemClickListener { _, _, position, _ ->
            when (position) {
                0 -> startActivity(Intent(this, ExamplesListActivity::class.java))
                1 -> startActivity(Intent(this, ScenesMenuActivity::class.java))
                2 -> startActivity(Intent(this, ParametersActivity::class.java))
                3 -> finishAffinity()
            }
        }
    }

    companion object {
        const val EXTRA_EXAMPLE_INDEX = "com.example.genesisv.EXAMPLE_INDEX"
    }
}
