package com.example.genesisv

import android.content.Intent
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.ListView
import androidx.appcompat.app.AppCompatActivity

/**
 * Pantalla de menú: lista de ejemplos del repo GenesisV (GitHub).
 * Al elegir uno se abre MainActivity (OpenGL) con ese índice.
 */
class MenuActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_menu)

        val listView = findViewById<ListView>(R.id.list_examples)
        val examples = listOf(
            "001: Triángulo rotando",
            "002: Cuadrado con colores",
            "003: Cubo en alambre (wireframe)",
            "004: Cubo sólido con colores",
            "005: Varios objetos rotando",
            "006: Cuadrado con textura de madera",
            "007: Cubo con textura",
            "008: Cubo con texturas distintas por cara",
            "009: Textura en movimiento (animada)",
            "010: Tipos de filtrado de textura",
            "011: Tiles desde una imagen grande",
            "012: Varios objetos con distintas texturas",
            "013: Texturas con iluminación",
            "014: Escena más compleja",
            "015: Efectos avanzados con texturas"
        )

        listView.adapter = ArrayAdapter(
            this,
            android.R.layout.simple_list_item_1,
            examples
        )

        listView.setOnItemClickListener { _, _, position, _ ->
            val intent = Intent(this, MainActivity::class.java).apply {
                putExtra(EXTRA_EXAMPLE_INDEX, position + 1)
            }
            startActivity(intent)
        }
    }

    companion object {
        const val EXTRA_EXAMPLE_INDEX = "com.example.genesisv.EXAMPLE_INDEX"
    }
}
