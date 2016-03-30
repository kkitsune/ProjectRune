/* Copyright (c) 2015 Jeramy Harrison
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Modified by Daniel Grondin <tm.kkitsune@gmail.com>
 */

#pragma once

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <memory>
#include <vector>
#include <tuple>
#include <list>

namespace ginseng
{
	namespace _detail
	{
		using namespace std;

		// GUID

		using GUID = int_fast64_t;

		inline GUID& instGUID()
		{
			static GUID guid = 0;
			return guid;
		}

		inline GUID nextGUID()
		{
			GUID rv = ++instGUID();
			if (rv == numeric_limits<GUID>::max())
				throw;
			return rv;
		}

		template<typename T>
		GUID getGUID()
		{
			static GUID guid = nextGUID();
			return guid;
		}

		// Component
		template<typename T>
		class Component
		{
			T _val;

			Component(Component const&) = delete;

			Component(Component&&) noexcept = delete;

			Component& operator=(Component const&) = delete;

			Component& operator=(Component&&) noexcept = delete;

		public:
			Component(T t) : _val(move(t))
			{ }

			T& val()
			{
				return _val;
			}
		};

		using AbstractComponent = void;

		// GUIDPair
		template<typename T>
		class GUIDPair
		{
			pair<GUID, T> _val;

		public:
			GUIDPair(GUID guid, T t) : _val(guid, move(t))
			{ }

			GUID guid() const noexcept
			{
				return _val.first;
			}

			T& val() noexcept
			{
				return _val.second;
			}

			T const& val() const noexcept
			{
				return _val.second;
			}
		};

		template<typename T>
		bool operator<(GUIDPair<T> const& a, GUIDPair<T> const& b) noexcept
		{
			return a.guid() < b.guid();
		}

		template<typename T>
		bool operator<(GUIDPair<T> const& a, GUID guid) noexcept
		{
			return a.guid() < guid;
		}

		// Entity
		using ComponentData = GUIDPair<shared_ptr < AbstractComponent>>;

		class Entity
		{
			template<template<typename> class AllocatorT>
			friend class Database;

			using ComponentVec = vector<ComponentData>;

			ComponentVec components;

		public:
			Entity() = default;

			Entity(Entity const&) = delete;

			Entity(Entity&&) noexcept = default;

			Entity& operator=(Entity const&) = delete;

			Entity& operator=(Entity&&) = default;
		};

		// Queries
		template<typename T>
		struct Not
		{ };

		template<typename T>
		struct Tag
		{ };

		// Traits
		struct ComponentTags
		{
			struct normal
			{ };
			struct tagged
			{ };
			struct inverted
			{ };
			struct info
			{ };
			struct eid
			{ };
		};

		template<typename DB, typename Component, template<typename> class ComInfo = DB::template ComInfo>
		struct ComponentTraits
		{
			using tag = ComponentTags::normal;
			using com = Component;
		};

		template<typename DB, typename Component, template<typename> class ComInfo>
		struct ComponentTraits<DB, ComInfo<Component>, ComInfo>
		{
			using tag = ComponentTags::info;
			using com = Component;
		};

		template<typename DB, typename Component, template<typename> class ComInfo>
		struct ComponentTraits<DB, Tag<Component>, ComInfo>
		{
			using tag = ComponentTags::tagged;
			using com = Tag<Component>;
		};

		template<typename DB, typename Component, template<typename> class ComInfo>
		struct ComponentTraits<DB, Not<Component>, ComInfo>
		{
			using tag = ComponentTags::inverted;
			using com = Component;
		};

		template<typename DB, template<typename> class ComInfo>
		struct ComponentTraits<DB, typename DB::EntID, ComInfo>
		{
			using tag = ComponentTags::eid;
		};

		template<typename DB, typename EntID, typename... Components>
		struct Applier;

		template<typename DB, typename EntID, typename HeadCom, typename... TailComs>
		struct Applier<DB, EntID, HeadCom, TailComs...>
		{
			template<typename Traits, typename Visitor, typename... Args>
			static void helper(ComponentTags::normal, EntID eid, Visitor&& visitor, Args&& ... args)
			{
				if (auto com_info = eid.template get<typename Traits::com>())
					return Applier<DB, EntID, TailComs...>::try_apply(eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., com_info.data());
			}

			template<typename Traits, typename Visitor, typename... Args>
			static void helper(ComponentTags::inverted, EntID eid, Visitor&& visitor, Args&& ... args)
			{
				if (auto com_info = eid.template get<typename Traits::com>())
					return;
				return Applier<DB, EntID, TailComs...>::try_apply(eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., Not<typename Traits::com>{});
			}

			template<typename Traits, typename Visitor, typename... Args>
			static void helper(ComponentTags::info, EntID eid, Visitor&& visitor, Args&& ... args)
			{
				if (auto com_info = eid.template get<typename Traits::com>())
					return Applier<DB, EntID, TailComs...>::try_apply(eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., com_info);
			}

			template<typename Traits, typename Visitor, typename... Args>
			static void helper(ComponentTags::tagged, EntID eid, Visitor&& visitor, Args&& ... args)
			{
				if (auto com_info = eid.template get<typename Traits::com>())
					return Applier<DB, EntID, TailComs...>::try_apply(eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., typename Traits::com{});
			}

			template<typename Traits, typename Visitor, typename... Args>
			static void helper(ComponentTags::eid, EntID eid, Visitor&& visitor, Args&& ... args)
			{
				return Applier<DB, EntID, TailComs...>::try_apply(eid, std::forward<Visitor>(visitor), std::forward<Args>(args)..., eid);
			}

			template<typename Visitor, typename... Args>
			static void try_apply(EntID eid, Visitor&& visitor, Args&& ... args)
			{
				using Traits = ComponentTraits<DB, HeadCom>;
				return helper<Traits>(typename Traits::tag{}, eid, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
			}
		};

		template<typename DB, typename EntID>
		struct Applier<DB, EntID>
		{
			template<typename Visitor, typename... Args>
			static void try_apply(EntID eid, Visitor&& visitor, Args&& ... args)
			{
				(void) eid;
				std::forward<Visitor>(visitor)(std::forward<Args>(args)...);
			}
		};

		template<typename DB, typename... Components>
		struct VisitorTraitsImpl
		{
			using EntID = typename DB::EntID;

			template<typename Visitor>
			static void apply(EntID eid, Visitor&& visitor)
			{
				Applier<DB, EntID, Components...>::try_apply(eid, std::forward<Visitor>(visitor));
			}
		};

		template<typename DB, typename Visitor>
		struct VisitorTraits : VisitorTraits<DB, decltype(&Visitor::operator())>
		{ };

		template<typename DB, typename R, typename... Ts>
		struct VisitorTraits<DB, R(*)(Ts...)> : VisitorTraitsImpl<DB, std::decay_t < Ts> ...>
		{ };

		template<typename DB, typename Visitor, typename R, typename... Ts>
		struct VisitorTraits<DB, R(Visitor::*)(Ts...)> : VisitorTraitsImpl<DB, std::decay_t < Ts> ...>
		{ };

		template<typename DB, typename Visitor, typename R, typename... Ts>
		struct VisitorTraits<DB, R(Visitor::*)(Ts...) const> : VisitorTraitsImpl<DB, std::decay_t < Ts> ...>
		{ };

		template<typename DB, typename Visitor, typename R, typename... Ts>
		struct VisitorTraits<DB, R(Visitor::*)(Ts...)&> : VisitorTraitsImpl<DB, std::decay_t < Ts> ...>
		{ };

		template<typename DB, typename Visitor, typename R, typename... Ts>
		struct VisitorTraits<DB, R(Visitor::*)(Ts...) const&> : VisitorTraitsImpl<DB, std::decay_t < Ts> ...>
		{ };

		template<typename DB, typename Visitor, typename R, typename... Ts>
		struct VisitorTraits<DB, R(Visitor::*)(Ts...)&&> : VisitorTraitsImpl<DB, std::decay_t < Ts> ...>
		{ };

		/*! Database
		 *
		 * An Entity component Database. Uses the given allocator to allocate
		 * components, and may also use the same allocator for internal data.
		 *
		 * @warning
		 * This container does not perform any synchronization. Therefore, it is not
		 * considered "thread-safe".
		 *
		 * @tparam AllocatorT Component allocator.
		 */
		template<template<typename> class AllocatorT = allocator>
		class Database
		{
			template<typename T>
			using AllocList = list<T, AllocatorT<T>>;

			AllocList<Entity> entities;

		public:
			// IDs
			// forward declarations needed for EntID

			class ComID;

			template<typename Com>
			class ComInfo;

			/*! Entity ID
			 *
			 * A handle to an Entity. Very lightweight.
			 */
			class EntID
			{
				friend class Database;

				typename AllocList<Entity>::const_iterator iter;

			public:
				/*! Query the Entity for a component.
				 *
				 * Queries the Entity for the given component type.
				 * If found, returns a valid ComInfo object for the requested
				 * component.
				 * Otherwise, returns an invalid ComInfo object.
				 *
				 * @tparam T Explicit type of component.
				 * @return ComInfo for the requested component.
				 */
				template<typename T>
				ComInfo<T> get() const
				{
					ComID cid;

					GUID guid = getGUID<T>();
					auto& comvec = iter->components;

					auto pos = lower_bound(begin(comvec), end(comvec), guid);

					cid.eid = *this;
					cid.iter = pos;

					if (pos != end(comvec) && pos->guid() == guid)
						return {cid};

					return {};
				}

				/*! Query the Entity for multiple components.
				 *
				 * Returns a tuple containing results equivalent to multiple
				 * calls to get().
				 *
				 * For example, if `eid.getComs<X,Y,Z>()` is called, it is
				 * equivalent to calling
				 * `std::make_tuple(get<X>(),get<Y>(),get<Z>())`.
				 *
				 * @tparam Ts Explicit component types.
				 * @return Tuple of results equivalent to get().
				 */
				template<typename... Ts>
				tuple<ComInfo<Ts>...> coms() const
				{
					return make_tuple(get<Ts>()...);
				}

				/*! Compares this EntID to another for equivalence.
				 *
				 * Returns true only if the two EntIDs are handles to the same
				 * Entity.
				 *
				 * @param other The EntID to compare to this.
				 * @return True if EntIDs are equivalent.
				 */
				bool operator==(EntID const& other) const
				{
					return (iter == other.iter);
				}

				/*! Compares this EntID to another for ordering.
				 *
				 * Provides a strict weak ordering for EntIDs.
				 *
				 * @param other The EntID to compare to this.
				 * @return True if this should be ordered before other.
				 */
				bool operator<(EntID const& other) const
				{
					return (&*iter < &*other.iter);
				}
			};

			/*! Component ID
			 *
			 * A handle to a type-erased component. Very lightweight.
			 */
			class ComID
			{
				friend class Database;

				EntID eid;
				Entity::ComponentVec::const_iterator iter;

			public:
				/*! Access component data.
				 *
				 * Reverses type erasure on the component's data.
				 * The specified type must match the component's real type,
				 * otherwise behaviour is undefined.
				 *
				 * @tparam Explicit component data type.
				 * @return Reference to component data.
				 */
				template<typename T>
				T& cast() const
				{
					auto& sptr = iter->val();
					auto ptr = static_cast<Component<T>*>(sptr.get());
					return ptr->val();
				}

				/*! Get parent's EntID.
				 *
				 * Returns a handle to the parent Entity.
				 *
				 * @return Handle to parent Entity.
				 */
				EntID const& EID() const
				{
					return eid;
				}

				/*! Compares this ComID to another for equivalence.
				 *
				 * Returns true only if the two ComIDs are handles to the same
				 * Component.
				 *
				 * @param other The ComID to compare to this.
				 * @return True if ComIDs are equivalent.
				 */
				bool operator==(ComID const& other) const
				{
					return (iter == other.iter);
				}

				/*! Compares this ComID to another for ordering.
				 *
				 * Provides a strict weak ordering for ComIDs.
				 *
				 * @param other The ComID to compare to this.
				 * @return True if this should be ordered before other.
				 */
				bool operator<(ComID const& other) const
				{
					return less<decltype(&*iter)> {}(&*iter, &*other.iter);
				}
			};

			/*! Component Info
			 *
			 * A handle to a component of known type.
			 * Provides direct access to the component,
			 * as well as its ComID.
			 *
			 * @tparam Com Component type.
			 */
			template<typename Com>
			class ComInfo
			{
				friend class Database;

				Com* ptr = nullptr;
				ComID cid;

				ComInfo(ComID i) : ptr(&i.template cast<Com>()), cid(i)
				{ }

			public:
				using type = Com;

				ComInfo() = default;

				/*! Test for validity.
				 *
				 * True if this ComInfo points to a component.
				 * False otherwise.
				 *
				 * @return True if valid.
				 */
				explicit operator bool() const
				{
					return ptr != nullptr;
				}

				/*! Get component.
				 *
				 * @warning
				 * Behaviour is undefined if this ComInfo is invalid.
				 *
				 * @return The component.
				 */
				Com& data() const
				{
					return *ptr;
				}

				/*! Get component ID.
				 *
				 * @warning
				 * Behaviour is undefined if this ComInfo is invalid.
				 *
				 * @return A ComID handle for this component.
				 */
				ComID const& id() const
				{
					return cid;
				}
			};

			/*! Component Info for Tags
			 *
			 * A handle to a component of known type.
			 * Provides the component's ComID.
			 *
			 * @tparam Com Component type.
			 */
			template<typename Com>
			class ComInfo<Tag<Com>>
			{
				friend class Database;

				bool is_valid = false;
				ComID cid;

				ComInfo(ComID i) : is_valid(true), cid(i)
				{ }

			public:
				using type = Com;

				ComInfo() = default;

				/*! Test for validity.
				 *
				 * True if this ComInfo points to a component.
				 * False otherwise.
				 *
				 * @return True if valid.
				 */
				explicit operator bool() const
				{
					return is_valid;
				}

				/*! Get component ID.
				 *
				 * @warning
				 * Behaviour is undefined if this ComInfo is invalid.
				 *
				 * @return A ComID handle for this component.
				 */
				ComID const& id() const
				{
					return cid;
				}
			};

			// Entity functions

			/*! Creates a new Entity.
			 *
			 * Creates a new Entity that has no components.
			 *
			 * @return EntID of the new Entity.
			 */
			EntID create_entity()
			{
				EntID rv;
				rv.iter = entities.emplace(end(entities));
				return rv;
			}

			/*! Destroys an Entity.
			 *
			 * Destroys the given Entity and all associated components.
			 *
			 * @warning
			 * All components associated with the Entity are destroyed.
			 * This means that all references and ComIDs associated with those
			 * components are invalidated.
			 *
			 * @param eid EntID of the Entity to erase.
			 */
			void erase_entity(EntID eid)
			{
				entities.erase(eid.iter);
			}

			/*! Emplace an Entity into this Database.
			 *
			 * Move the given Entity into this Database.
			 *
			 * @param ent Entity rvalue to move.
			 * @return EntID to the new Entity.
			 */
			EntID emplace_entity(Entity&& ent)
			{
				EntID rv;
				rv.iter = entities.emplace(end(entities), move(ent));
				return rv;
			}

			/*! Displace an Entity out of this Database.
			 *
			 * Moves the given Entity out of this Database.
			 *
			 * @warning
			 * All EntIDs associated with the Entity are invalidated.
			 *
			 * @param eid EntID to Entity to displace.
			 * @return Entity value.
			 */
			Entity displace_entity(EntID eid)
			{
				Entity rv = move(*entities.erase(eid.iter, eid.iter));
				entities.erase(eid.iter);
				return rv;
			}

			// Component functions

			/*! Create new component.
			 *
			 * Creates a new component from the given value and associates it with
			 * the given Entity.
			 * If a component of the same type already exists, it will be
			 * overwritten.
			 *
			 * @warning
			 * All ComIDs associated with components of the given Entity will be
			 * invalidated.
			 *
			 * @param eid Entity to attach new component to.
			 * @param com Component value.
			 * @return ComInfo for the new component.
			 */
			template<typename T>
			ComInfo<T> create_component(EntID eid, T com)
			{
				ComID cid;
				GUID guid = getGUID<T>();
				AllocatorT<Component<T>> alloc;

				auto& comvec = entities.erase(eid.iter, eid.iter)->components;
				auto pos = lower_bound(begin(comvec), end(comvec), guid);
				cid.eid = eid;

				if (pos != end(comvec) && pos->guid() == guid)
				{
					cid.iter = pos;
					cid.template cast<T>() = move(com);
				}
				else
				{
					auto ptr = allocate_shared<Component<T>>(alloc, move(com));
					cid.iter = comvec.emplace(pos, guid, move(ptr));
				}

				return {cid};
			}

			/*! Create new component.
			 *
			 * Creates a new component from the given value and associates it with
			 * the given Entity.
			 * If a component of the same type already exists, it will be
			 * overwritten.
			 *
			 * @warning
			 * All ComIDs associated with components of the given Entity will be
			 * invalidated.
			 *
			 * @param eid Entity to attach new component to.
			 * @param com Component value.
			 * @return ComInfo for the new component.
			 */
			template<typename T>
			ComInfo<Tag<T>> create_component(EntID eid, Tag<T> com)
			{
				ComID cid;
				GUID guid = getGUID<Tag<T>>();

				auto& comvec = entities.erase(eid.iter, eid.iter)->components;
				auto pos = lower_bound(begin(comvec), end(comvec), guid);
				cid.eid = eid;

				if (pos != end(comvec) && pos->guid() == guid)
					cid.iter = pos;
				else
					cid.iter = comvec.emplace(pos, guid, nullptr);

				return {cid};
			}

			/*! Erase a component.
			 *
			 * Destroys the given component and disassociates it from its Entity.
			 *
			 * @warning
			 * All ComIDs associated with components of the component's Entity will
			 * be invalidated.
			 *
			 * @param cid ComID of the component to erase.
			 */
			void erase_component(ComID cid)
			{
				auto& comvec = entities.erase(cid.eid.iter, cid.eid.iter)->components;
				comvec.erase(cid.iter);
			}

			/*! Emplace component data into this Database.
			 *
			 * Moves the given component data into this Database and associates it
			 * with the given Entity.
			 *
			 * @warning
			 * All ComIDs of components associated with the given Entity are
			 * invalidated.
			 *
			 * @param eid Entity to attach component to.
			 * @param dat Component data to move.
			 * @return ComID to the new component.
			 */
			ComID emplace_component(EntID eid, ComponentData&& dat)
			{
				ComID rv;
				auto& comvec = entities.erase(eid.iter, eid.iter)->components;
				GUID guid = dat.guid();

				auto pos = lower_bound(begin(comvec), end(comvec), dat);

				rv.eid = eid;

				if (pos != end(comvec) && pos->guid() == guid)
				{
					rv.iter = pos;
					pos->val() = move(dat.val());
				}
				else
					rv.iter = comvec.emplace(pos, move(dat));

				return rv;
			}

			/*! Displace a component out of this Database.
			 *
			 * Moves the given component out of this Database.
			 *
			 * @warning
			 * All ComIDs associated with the Entity associated with the given
			 * component are invalidated.
			 *
			 * @param cid ComID of the component to displace.
			 * @return Component data.
			 */
			ComponentData displace_component(ComID cid)
			{
				auto& comvec = entities.erase(cid.eid.iter, cid.eid.iter)->components;
				ComponentData rv = move(*comvec.erase(cid.iter, cid.iter));
				comvec.erase(cid.iter);
				return rv;
			}

			template<typename Visitor>
			void visit(Visitor&& visitor)
			{
				using Traits = VisitorTraits<Database, Visitor>;

				// Query loop
				for (auto i = begin(entities), e = end(entities); i != e; ++i)
				{
					EntID eid;
					eid.iter = i;
					Traits::apply(eid, visitor);
				}
			}

			template<typename Visitor>
			void visit(Visitor&& visitor) const
			{
				using Traits = VisitorTraits<Database, Visitor>;

				// Query loop
				for (auto i = begin(entities), e = end(entities); i != e; ++i)
				{
					EntID eid;
					eid.iter = i;
					Traits::apply(eid, visitor);
				}
			}

			// query

			/*! Query the Database.
			 *
			 * Queries the Database for Entities that match the given template parameters.
			 *
			 * Returns a `std::vector<std::tuple<Ts...>>` where each element is a tuple of values filled by calling
			 * `visit([](Ts...){})` and forwarding the visitor's parameters to each tuple.
			 *
			 * There will be one element in the returned vector for each entity visited.
			 *
			 * @tparam Ts Query parameters.
			 * @return Query results.
			 */
			template<typename... Ts>
			std::vector <std::tuple<Ts...>> query()
			{
				std::vector <std::tuple<Ts...>> rv;
				visit([&](Ts... params)
				      {
					      rv.emplace_back(std::forward<Ts>(params)...);
				      });
				return rv;
			}

			// status functions
			auto size() const -> decltype(entities.size())
			{
				return entities.size();
			}
		};

	} // namespace _detail

	using _detail::ComponentData;
	using _detail::Entity;
	using _detail::Database;
	using _detail::Not;
	using _detail::Tag;
} // namespace ginseng
