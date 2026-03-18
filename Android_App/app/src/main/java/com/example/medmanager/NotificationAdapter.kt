package com.gland.medmanager

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import java.text.SimpleDateFormat
import java.util.*

class NotificationAdapter(
    private val items: List<NotificationEntry>
) : RecyclerView.Adapter<NotificationAdapter.ViewHolder>() {

    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val icon: ImageView = view.findViewById(R.id.icon)
        val title: TextView = view.findViewById(R.id.title)
        val time: TextView = view.findViewById(R.id.time)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.item_notification, parent, false)
        return ViewHolder(view)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val entry = items[position]
        val type = NotificationMapper.fromFirebaseMessage(entry.message)

        holder.title.text =
            if (type == NotificationType.DEFAULT) entry.message else type.uiTitle

        holder.icon.setImageResource(type.iconRes)
        holder.icon.setColorFilter(
            holder.itemView.context.getColor(type.colorRes)
        )

        val formatter = SimpleDateFormat("dd MMM yyyy, HH:mm", Locale.getDefault())
        holder.time.text = formatter.format(Date(entry.timestamp))
    }



    override fun getItemCount(): Int = items.size
}
